#include <SPI.h>

#include <LoRa.h>
#include <SPI.h>
#include <VL53L4CD.h>
#include <Wire.h>
#include <avr/sleep.h>
#include <avr/wdt.h>

/** PROJECT: Wireless Welder Pedal v1.0 POWER-MASTER | TRANSMITTER UNIT */

// =============================================================
//                    USER CONFIGURATION
// =============================================================
const unsigned long STANDBY_MIN = 1;     // Minutes before Laser Standby
const unsigned long DEEP_SLEEP_MIN = 2; // Minutes before Deep Sleep
const unsigned long WAKE_UP_SAFE_TIME =
    1; // SECONDS to ignore trigger after wake up
const int LORA_TX_POWER =
    5; // Tx Power (2 to 20 dBm). 5 is recommended for workshop.

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
struct __attribute__((packed)) PedalData {
  uint16_t laserDist;
  bool switchClosed;
  uint16_t batV;
};
PedalData myPedal = {150, false, 0}; // 150mm is the default inactive distance
bool laserInitialized = false, isSleeping = false, powerState = true,
     laserRunning = false;
unsigned long lastActivityTime = 0, lastTxTime = 0, lastHeartbeat = 0;
unsigned long wakeUpTime = 0;
uint16_t buffer[3] = {
    150, 150, 150}; // Initializing buffer with default inactive distance
int bufIdx = 0;
float smoothedDist = 150.0; // Initializing smoothed distance with 150mm

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
                          // (preserves prescaler)
  powerState = false;
  isSleeping = false;
}

void wakeSystem() {
  powerState = true;
  ADCSRA |= (1 << ADEN); // Re-enable ADC preserving original Arduino prescaler
                         // (prevents 4MHz ADC clock bug)
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
  attachInterrupt(
      digitalPinToInterrupt(PIN_MODE), []() {}, LOW);
  PCMSK0 |= (1 << PCINT7);
  PCICR |= (1 << PCIE0);
  sei();
  sleep_cpu();
  sleep_disable();
  detachInterrupt(digitalPinToInterrupt(PIN_MODE));
  PCICR &= ~(1 << PCIE0); // Disable Pin Change Interrupts so they don't fire
                          // during normal operation
}

EMPTY_INTERRUPT(
    PCINT0_vect); // Handle wake-up interrupt safely without crashing

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

  // Activity Update - Independiente del USB (se dormirá aunque esté cargando)
  if (btn || rawTrigger || (laserInitialized && myPedal.laserDist < (140))) {
    lastActivityTime = millis();
  }

  // Detección de conexión USB en caliente para el IDE
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
    myPedal.laserDist = 999;
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

    ADCSRA &=
        ~(1 << ADEN); // Ensure ADC is OFF during sleep (preserves prescaler)
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
    sleep_enable();
    sei();
    sleep_cpu(); // MCU duerme aquí por 2s o hasta pisar el pedal
    sleep_disable();
    ADCSRA |=
        (1
         << ADEN); // Re-enable ADC for battery measurement preserving prescaler

    PCICR &= ~(1 << PCIE0);
    WDTCSR &= ~_BV(WDIE); // Desactivar WDT

    // Si despertó por WDT y el pedal no está presionado, enviar heartbeat
    if (digitalRead(PIN_SWITCH) != LOW) {
      Serial.println(F("TX: Standby Heartbeat Sent"));
      lastHeartbeat = millis();
      digitalWrite(PIN_LED, HIGH);
      myPedal.laserDist = 999;
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

  // Laser Reading
  myPedal.switchClosed = trigger;
  if (laserRunning) {
    sensor.read();
    if (!sensor.timeoutOccurred()) {
      buffer[bufIdx] = sensor.ranging_data.range_mm;
      bufIdx = (bufIdx + 1) % 3;
      uint16_t s[] = {buffer[0], buffer[1], buffer[2]};
      if (s[0] > s[1]) {
        uint16_t t = s[0];
        s[0] = s[1];
        s[1] = t;
      }
      if (s[1] > s[2]) {
        uint16_t t = s[1];
        s[1] = s[2];
        s[2] = t;
      }
      if (s[0] > s[1]) {
        uint16_t t = s[0];
        s[0] = s[1];
        s[1] = t;
      }
      uint16_t raw = s[1];
      uint16_t diff = abs((int)raw - (int)smoothedDist);
      if (diff > 3)
        smoothedDist = (float)raw;
      else
        smoothedDist = (0.15 * (float)raw) + (0.85 * smoothedDist);
      myPedal.laserDist = (uint16_t)smoothedDist;
    } else {
      myPedal.laserDist = 150; // Fallback to 150mm on error
    }
  } else {
    myPedal.laserDist = 150; // 150mm when laser is off
  }

  // Battery monitoring (500ms rate limit with 5ms quiet period)
  static unsigned long lastBatReadTx = 0;
  if (millis() - lastBatReadTx >= 500) {
    lastBatReadTx = millis();
    ADMUX = _BV(REFS0) | _BV(MUX4) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
    ADCSRA |= _BV(ADSC);
    while (bit_is_set(ADCSRA, ADSC))
      ;
    analogRead(PIN_BAT);
    delay(5); // Quiet period for accurate high-impedance reading
    myPedal.batV = analogRead(PIN_BAT);
  }

  static int lowBatteryCounter = 0;
  if (millis() > 10000 && myPedal.batV > 0 && myPedal.batV < 188) {
    if (++lowBatteryCounter > 50)
      sleepSystem();
  } else
    lowBatteryCounter = 0;

  if (millis() - lastTxTime > 33) { // Rate limit LoRa to ~30Hz
    lastTxTime = millis();
    LoRa.beginPacket();
    LoRa.write((uint8_t *)&myPedal, sizeof(PedalData));
    LoRa.endPacket();
  }
}