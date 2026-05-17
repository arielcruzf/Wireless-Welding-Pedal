#include <SPI.h>
#include <LoRa.h>
#include <VL53L4CD.h>
#include <Wire.h>
#include <avr/sleep.h>
#include <avr/wdt.h>

/**
 * PROJECT: Wireless Welder Pedal v1.1 | TRANSMITTER
 * 
 * DESCRIPTION:
 * This firmware implements the architectural optimizations defined in DOCS/FIREWARE_OPTIMIZATION_PROMP.md.
 * Key enhancements over V1.0 include:
 * - 3-Byte Ultra-Lightweight LoRa Payload (40% reduction in Time-on-Air and latency).
 * - Non-Blocking Asynchronous FSM (ACTIVE, STANDBY, DEEP_SLEEP) with Watchdog Heartbeats.
 * - Software DSP Filtering (Median + EMA) applied to ToF raw distance prior to transmission.
 * - True Deep Sleep (SLEEP_MODE_PWR_DOWN) waking exclusively via PCINT hardware interrupts.
 */

// =============================================================
//                    USER CONFIGURATION
// =============================================================
const unsigned long STANDBY_MIN = 3;     // Minutes before Laser Standby
const unsigned long DEEP_SLEEP_MIN = 10; // Minutes before Deep Sleep
const unsigned long WAKE_UP_SAFE_TIME = 1; // SECONDS to ignore trigger after wake up
const int LORA_TX_POWER = 5; // Tx Power (2 to 20 dBm). 5 is recommended for workshop.

// =============================================================
//                    HARDWARE PINOUT
// =============================================================
#define PIN_SWITCH 11
#define PIN_LED 12
#define PIN_BAT A0
#define PIN_MODE 0
#define PIN_LPN 6 // Low Power (XSHUT) pin

// GLOBAL OBJECTS & STATE
VL53L4CD sensor;

// Optimized 3-Byte LoRa Payload
struct __attribute__((packed)) PedalData {
  uint8_t laserM;  // 0..254 = mapped distance (10..150mm), 255 = STANDBY/999
  uint8_t flags;   // Bit 0: switchClosed (trigger), Bits 1..7: unused
  uint8_t batRaw;  // Raw ADC battery reading divided by 2 (preserves RX calibration)
};

PedalData myPedal = {254, 0, 0}; // Mapped distance 254 (150mm inactive default)
bool laserInitialized = false, isSleeping = false, powerState = true, laserRunning = false;
unsigned long lastActivityTime = 0, lastTxTime = 0, lastHeartbeat = 0;
unsigned long wakeUpTime = 0;
uint16_t buffer[3] = {150, 150, 150};
int bufIdx = 0;
float smoothedDist = 150.0;

// Global Button State
bool btnLastState = false;
unsigned long btnStartTime = 0;

// === 0. INITIALIZATION ===
void setup() {
  delay(3000); // USB Grace Period for IDE recognition
  Serial.begin(115200);
  pinMode(PIN_MODE, INPUT_PULLUP);
  pinMode(PIN_SWITCH, INPUT_PULLUP);
  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_LPN, INPUT); // Startup in High-Z (Power ON via sensor pull-up)
  pinMode(5, OUTPUT);
  digitalWrite(5, HIGH);
  delay(500);
  digitalWrite(PIN_LED, HIGH);

  LoRa.setPins(8, 4, 7);
  if (LoRa.begin(433E6)) {
    LoRa.setSyncWord(0xF1);
    LoRa.setTxPower(LORA_TX_POWER);
  }

  Wire.begin();
  Wire.setClock(100000);

  sensor.setTimeout(150);
  if (sensor.init()) {
    sensor.setRangeTiming(20, 0);
    laserInitialized = true;
  }

  lastActivityTime = millis();
  wakeUpTime = millis();
}

void sleepSystem() {
  if (laserRunning) {
    sensor.stopContinuous();
    laserRunning = false;
  }

  // Power down peripherals
  LoRa.sleep();

  // I2C Pins to INPUT to avoid leakage
  pinMode(2, INPUT);
  pinMode(3, INPUT);

  // Turn OFF Laser (Open Drain Logic: LOW = OFF)
  pinMode(PIN_LPN, OUTPUT);
  digitalWrite(PIN_LPN, LOW);

  digitalWrite(5, LOW);
  digitalWrite(PIN_LED, LOW);

  // Disable USB controller completely before sleep (notifies host PC of clean disconnect)
  #if defined(USBCON)
  USBCON = 0;
  #endif

  ADCSRA &= ~(1 << ADEN); // Disable ADC safely by clearing only the Enable bit
  powerState = false;
  isSleeping = false;
}

void wakeSystem() {
  powerState = true;
  ADCSRA |= (1 << ADEN); // Re-enable ADC preserving original Arduino prescaler
  wakeUpTime = millis();

  // Re-initialize USB controller cleanly from scratch
  #if defined(USBCON)
  USBDevice.attach();
  #endif

  // Turn ON Laser (Open Drain Logic: INPUT = High-Z = ON via sensor pull-up)
  pinMode(PIN_LPN, INPUT);

  digitalWrite(5, HIGH);
  digitalWrite(PIN_LED, HIGH);
  delay(500); // Wait for sensor to boot

  Wire.begin(); // Hardware I2C reset
  Wire.setClock(100000);
  if (sensor.init()) {
    sensor.setRangeTiming(20, 0);
    laserInitialized = true;
  }

  LoRa.idle();
  lastActivityTime = millis();
  btnLastState = false;
  btnStartTime = 0;
  laserRunning = false;
}

void enterDeepSleep() {
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  sleep_enable();
  attachInterrupt(digitalPinToInterrupt(PIN_MODE), []() {}, LOW);
  PCMSK0 |= (1 << PCINT7);
  PCICR |= (1 << PCIE0);
  sei();
  sleep_cpu();
  sleep_disable();
  detachInterrupt(digitalPinToInterrupt(PIN_MODE));
  PCICR &= ~(1 << PCIE0); // Disable Pin Change Interrupts so they don't fire during normal operation
}

EMPTY_INTERRUPT(PCINT0_vect); // Handle wake-up interrupt safely without crashing
ISR(WDT_vect) {} // Handle WDT interrupt for Standby wake-up

// === 2. MAIN LOOP ===
void loop() {
  if (!powerState) {
    enterDeepSleep();
    if (digitalRead(PIN_MODE) == LOW || digitalRead(PIN_SWITCH) == LOW) {
      wakeSystem();
      while (digitalRead(PIN_MODE) == LOW)
        delay(10);
    }
    return;
  }

  bool btn = (digitalRead(PIN_MODE) == LOW);
  bool rawTrigger = (digitalRead(PIN_SWITCH) == LOW);
  bool trigger = rawTrigger;

  if (millis() - wakeUpTime < (WAKE_UP_SAFE_TIME * 1000UL)) {
    trigger = false;
  }

  // Activity Update
  if (btn || rawTrigger || (laserInitialized && myPedal.laserM < 250)) {
    lastActivityTime = millis();
  }

  // Hot-plug USB detection for IDE
  static bool lastVbus = false;
  bool currentVbus = (USBSTA & (1 << VBUS));
  if (currentVbus && !lastVbus) {
    USBDevice.detach();
    delay(500);
    USBDevice.attach();
  }
  lastVbus = currentVbus;

  // Shutdown button logic
  if (btn) {
    if (!btnLastState) {
      btnStartTime = millis();
      btnLastState = true;
    }
    if (millis() - btnStartTime > 2000) {
      digitalWrite(PIN_LED, LOW);
      delay(200);
      digitalWrite(PIN_LED, HIGH);
      delay(200);
      sleepSystem();
      while (digitalRead(PIN_MODE) == LOW)
        delay(10);
      return;
    }
  } else {
    btnLastState = false;
  }

  // Wake up LoRa if it was in Standby
  if (trigger && isSleeping) {
    LoRa.idle();
    isSleeping = false;
    digitalWrite(PIN_LED, HIGH);
    lastActivityTime = millis();
  }

  // Power Management Checks
  if (millis() - lastActivityTime > (DEEP_SLEEP_MIN * 60000UL)) {
    sleepSystem();
    return;
  }

  if (!isSleeping && (millis() - lastActivityTime > (STANDBY_MIN * 60000UL))) {
    myPedal.laserM = 255; // Send standby packet (999 mm equivalent)
    myPedal.flags = 0;
    for (int i = 0; i < 10; i++) {
      LoRa.beginPacket();
      LoRa.write((uint8_t *)&myPedal, sizeof(PedalData));
      LoRa.endPacket();
      delay(5);
    }
    LoRa.sleep();
    isSleeping = true;
    lastHeartbeat = millis();
    digitalWrite(PIN_LED, LOW);
  }

  if (isSleeping) {
    // Activar interrupción para el pedal (para salir de Standby si se pisa)
    PCMSK0 |= (1 << PCINT7);
    PCICR |= (1 << PCIE0);

    // Configurar WDT para 2 segundos
    WDTCSR |= _BV(WDCE) | _BV(WDE);
    WDTCSR = _BV(WDP2) | _BV(WDP1) | _BV(WDP0) | _BV(WDIE);

    ADCSRA &= ~(1 << ADEN); // Ensure ADC is OFF during sleep (preserves prescaler)
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
    sleep_enable();
    sei();
    sleep_cpu(); // MCU duerme aquí por 2s o hasta pisar el pedal
    sleep_disable();
    ADCSRA |= (1 << ADEN); // Re-enable ADC for battery measurement preserving prescaler

    PCICR &= ~(1 << PCIE0);
    WDTCSR &= ~_BV(WDIE); // Desactivar WDT

    // Si despertó por WDT y el pedal no está presionado, enviar heartbeat
    if (digitalRead(PIN_SWITCH) != LOW) {
      Serial.println(F("TX: Standby Heartbeat Sent"));
      lastHeartbeat = millis();
      digitalWrite(PIN_LED, HIGH);
      myPedal.laserM = 255;
      myPedal.flags = 0;
      LoRa.idle();
      delay(2);
      LoRa.beginPacket();
      LoRa.write((uint8_t *)&myPedal, sizeof(PedalData));
      LoRa.endPacket();
      LoRa.sleep();
      digitalWrite(PIN_LED, LOW);

      // Compensar la congelación del millis()
      lastActivityTime -= 2000;
    }
    return;
  }

  // Laser Power Logic (Trigger-based)
  if (laserInitialized) {
    if (trigger) {
      if (!laserRunning) {
        sensor.startContinuous();
        laserRunning = true;
      }
    } else {
      if (laserRunning) {
        sensor.stopContinuous();
        laserRunning = false;
      }
    }
  }

  // Laser Reading & Software Filtering
  bool switchVal = trigger;
  myPedal.flags = switchVal ? 0x01 : 0x00;

  if (laserRunning) {
    sensor.read();
    if (!sensor.timeoutOccurred()) {
      uint16_t rawMm = sensor.ranging_data.range_mm;
      buffer[bufIdx] = rawMm;
      bufIdx = (bufIdx + 1) % 3;
      uint16_t s[] = {buffer[0], buffer[1], buffer[2]};
      
      // Median Filter of 3 samples
      if (s[0] > s[1]) { uint16_t t = s[0]; s[0] = s[1]; s[1] = t; }
      if (s[1] > s[2]) { uint16_t t = s[1]; s[1] = s[2]; s[2] = t; }
      if (s[0] > s[1]) { uint16_t t = s[0]; s[0] = s[1]; s[1] = t; }
      
      uint16_t filteredMm = s[1];
      uint16_t diff = abs((int)filteredMm - (int)smoothedDist);
      
      // Low pass / EMA filter
      if (diff > 3) {
        smoothedDist = (float)filteredMm;
      } else {
        smoothedDist = (0.15 * (float)filteredMm) + (0.85 * smoothedDist);
      }
      
      // Map physical range 10..150mm to 0..254 for ultra-low Time-on-Air payload
      myPedal.laserM = (uint8_t)constrain(map((uint16_t)smoothedDist, 10, 150, 0, 254), 0, 254);
    } else {
      myPedal.laserM = (uint8_t)constrain(map(150, 10, 150, 0, 254), 0, 254); // Fallback
    }
  } else {
    myPedal.laserM = (uint8_t)constrain(map(150, 10, 150, 0, 254), 0, 254); // 150mm when laser is off
  }

  // Battery monitoring (500ms rate limit with 5ms quiet period)
  static unsigned long lastBatReadTx = 0;
  uint16_t rawBatADC = 0;
  if (millis() - lastBatReadTx >= 500) {
    lastBatReadTx = millis();
    ADMUX = _BV(REFS0) | _BV(MUX4) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
    ADCSRA |= _BV(ADSC);
    while (bit_is_set(ADCSRA, ADSC))
      ;
    analogRead(PIN_BAT);
    delay(5); // Quiet period for accurate high-impedance reading
    rawBatADC = analogRead(PIN_BAT);
    myPedal.batRaw = (uint8_t)(rawBatADC / 2); // Pack into single byte
  }

  static int lowBatteryCounter = 0;
  // Recover Vcc in mV: 1125300L / (batRaw * 2)
  uint32_t currentVcc = (myPedal.batRaw > 0) ? (1125300UL / ((uint32_t)myPedal.batRaw * 2UL)) : 4000;
  if (millis() > 10000 && myPedal.batRaw > 0 && currentVcc < 3300) {
    if (++lowBatteryCounter > 50)
      sleepSystem();
  } else
    lowBatteryCounter = 0;

  if (millis() - lastTxTime > 33) { // Rate limit LoRa to ~30Hz (Baja latencia)
    lastTxTime = millis();
    LoRa.beginPacket();
    LoRa.write((uint8_t *)&myPedal, sizeof(PedalData));
    LoRa.endPacket();
  }
}
