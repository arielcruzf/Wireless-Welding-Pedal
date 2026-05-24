#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <EEPROM.h>
#include <LoRa.h>
#include <SPI.h>
#include <Wire.h>
#include <avr/sleep.h>
#include <avr/wdt.h>

// Disable Watchdog Timer immediately on boot to prevent bootloop on ATmega32U4
void disable_wdt_early(void) __attribute__((naked, section(".init3")));
void disable_wdt_early(void) {
  MCUSR = 0;
  wdt_disable();
}

/**
 * PROJECT: Wireless Welder Pedal v1.1 | RECEIVER
 * 
 * DESCRIPTION:
 * This firmware implements the architectural optimizations defined in DOCS/FIREWARE_OPTIMIZATION_PROMP.md.
 * Key enhancements over V1.0 include:
 * - 3-Byte Ultra-Lightweight LoRa Payload (40% reduction in Time-on-Air and latency).
 * - Linear mapping decompression of ToF distance (0..254 -> 10..150mm) and Standby flags.
 * - Hardware-compatible Battery ADC scaling (divided by 2 over-the-air, multiplied by 2 locally).
 * - Native preservation of V1.0 Failsafe, OLED, and EMA smoothing layers for maximum stability.
 */

// =============================================================
//                    USER CONFIGURATION
// =============================================================
const long FAILSAFE_LIMIT = 1000;        // Max wait time for signal
const unsigned long DEEP_SLEEP_MIN = 15; // Minutes before Deep Sleep
const int PEDAL_UP_MM = 60;              // Pedal up distance
const int PEDAL_DOWN_MM = 18;            // Pedal down distance
const uint32_t RX_BATTERY_CALIBRATION = 17850; // Physical battery voltage calibration for Receiver
const uint32_t TX_BATTERY_CALIBRATION = 23800; // Physical battery voltage calibration for Transmitter

// =============================================================
//                    HARDWARE PINOUT
// =============================================================
#define PIN_PWM 9
#define PIN_PWM_GND 6
#define PIN_REL1 A5
#define PIN_REL1_GND 10
#define PIN_REL2 A3
#define PIN_REL2_GND 12
#define PIN_BAT A0
#define PIN_MODE 0
#define PIN_MODE_GND 1
#define PIN_LED A1
#define PIN_LED_GND A2
#define PIN_BUZZER A4

#define EEPROM_ADDR_MODE 0
#define PWM_MAX_VAL 244 // System hardware limit for active welding PWM
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// GLOBAL VARIABLES
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Optimized 3-Byte LoRa Payload
struct __attribute__((packed)) PedalDataPayload {
  uint8_t laserM;  // 0..254 = mapped distance (10..150mm), 255 = STANDBY (999)
  uint8_t flags;   // Bit 0: switchClosed (trigger), Bits 1..7: unused
  uint8_t batRaw;  // Raw ADC battery reading divided by 4
};

// Reconstructed/Compatible struct to keep remaining V1.0 logic untouched
struct PedalData {
  uint16_t laserDist;
  bool switchClosed;
  uint16_t batV;
};

PedalData myPedal;
uint16_t localBatV = 0;
int lastRSSI = 0, currentPWM = 0, currentPwmMin = 0;
uint16_t lastValidLaserDist = 60; // Last laser distance recorded before shutdown
unsigned long lastReception = 0, lastActivity = 0;
float sT = 0, sR = 0; // Global battery smoothing variables to allow reset on wake
bool systemLocked = true, isStandby = false, outputsEnabled = false, powerState = true;

enum class SystemState {
  STARTING,
  SEARCHING,
  STANDBY,
  CONNECTED,
  DISCONNECTED,
  HOLDING
};
SystemState currentState = SystemState::STARTING;

// ASSETS
const unsigned char PROGMEM antenna_bitmap[] = {
    0x01, 0x00, 0x03, 0x80, 0x03, 0x80, 0x1b, 0xb0, 0x3b, 0xb8, 0x73, 0x9c,
    0x63, 0x8c, 0xe3, 0x8c, 0xc1, 0x06, 0xc0, 0x06, 0xc0, 0x06, 0xe0, 0x0e,
    0x60, 0x0c, 0x70, 0x18, 0x3c, 0x78, 0x1f, 0xf0, 0x07, 0xc0};
const unsigned char PROGMEM flash_bitmap[] = {
    0x00, 0x80, 0x00, 0x00, 0x80, 0x00, 0x01, 0x80, 0x00, 0x01, 0x80,
    0x00, 0x03, 0x80, 0x00, 0x03, 0x80, 0x00, 0x07, 0x80, 0x00, 0x07,
    0xfc, 0x00, 0x0f, 0xf8, 0x00, 0x0f, 0xf8, 0x00, 0x1f, 0xf0, 0x00,
    0x00, 0xf0, 0x00, 0x00, 0xe0, 0x00, 0x00, 0xe0, 0x00, 0x00, 0xc0,
    0x00, 0x00, 0xc0, 0x00, 0x00, 0x80, 0x00, 0x00, 0x80, 0x00};

// 'PBT Icon' (TX - PEDAL BATTERY), 26x17px
const unsigned char PROGMEM icon_TX[] = {
    0x00, 0x00, 0x00, 0x00, 0x1f, 0x0f, 0x8f, 0xc0, 0x1f, 0x8f, 0xcf, 0xc0,
    0x19, 0x8c, 0xc3, 0x00, 0x19, 0x8c, 0xc3, 0x00, 0x19, 0x8c, 0xc3, 0x00,
    0x19, 0x8c, 0xc3, 0x00, 0x1f, 0x8f, 0x83, 0x00, 0x1f, 0x0f, 0x83, 0x00,
    0x18, 0x0c, 0xc3, 0x00, 0x18, 0x0c, 0xc3, 0x00, 0x18, 0x0c, 0xc3, 0x00,
    0x18, 0x0c, 0xc3, 0x00, 0x18, 0x0f, 0xc3, 0x00, 0x18, 0x0f, 0x83, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

// 'RBT Icon' (RX - RECEIVER BATTERY), 26x17px
const unsigned char PROGMEM icon_RX[] = {
    0x00, 0x00, 0x00, 0x00, 0x1f, 0x0f, 0x8f, 0xc0, 0x1f, 0x8f, 0xcf, 0xc0,
    0x19, 0x8c, 0xc3, 0x00, 0x19, 0x8c, 0xc3, 0x00, 0x19, 0x8c, 0xc3, 0x00,
    0x19, 0x8c, 0xc3, 0x00, 0x1f, 0x0f, 0x83, 0x00, 0x1f, 0x8f, 0x83, 0x00,
    0x19, 0x8c, 0xc3, 0x00, 0x19, 0x8c, 0xc3, 0x00, 0x19, 0x8c, 0xc3, 0x00,
    0x19, 0x8c, 0xc3, 0x00, 0x19, 0x8f, 0xc3, 0x00, 0x19, 0x8f, 0x83, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

byte currentMode = 0;

class OutputPin {
  uint8_t pin;

public:
  OutputPin(uint8_t p) : pin(p) {}
  void begin() {
    pinMode(pin, OUTPUT);
    writeDigital(LOW);
  }
  void writeDigital(bool state) { digitalWrite(pin, state); }
  void writePWM(int val) { analogWrite(pin, val); }
};

class BuzzerAlarm {
  OutputPin buzzerPin;
  int buzzesRemaining = 0;
  bool isBuzzing = false;
  unsigned long lastToggleTime = 0;

public:
  BuzzerAlarm(uint8_t p) : buzzerPin(p) {}
  void begin() { buzzerPin.begin(); }

  void trigger(int count) {
    if (buzzesRemaining > 0)
      return;                    // Don't override ongoing alarm
    buzzesRemaining = count * 2; // Each buzz is an ON and an OFF phase
    isBuzzing = true;
    buzzerPin.writeDigital(HIGH);
    lastToggleTime = millis();
    buzzesRemaining--;
  }

  void update() {
    if (buzzesRemaining > 0) {
      if (millis() - lastToggleTime >= 1000) {
        lastToggleTime = millis();
        isBuzzing = !isBuzzing;
        buzzerPin.writeDigital(isBuzzing);
        buzzesRemaining--;
        if (buzzesRemaining == 0) {
          buzzerPin.writeDigital(LOW);
          isBuzzing = false;
        }
      }
    } else if (isBuzzing) {
      buzzerPin.writeDigital(LOW);
      isBuzzing = false;
    }
  }
};

OutputPin rel1(PIN_REL1);
OutputPin rel2(PIN_REL2);
OutputPin pwmOut(PIN_PWM);
BuzzerAlarm sysBuzzer(PIN_BUZZER);

// === 0. INITIALIZATION ===
void setup() {
  Serial.begin(115200);
  pinMode(PIN_LED, OUTPUT);
  digitalWrite(PIN_LED, HIGH);
  pinMode(PIN_LED_GND, OUTPUT);
  digitalWrite(PIN_LED_GND, LOW); // Virtual GND for Power Button LED
  pwmOut.begin();
  rel1.begin();
  rel2.begin();
  sysBuzzer.begin();
  pinMode(PIN_MODE, INPUT_PULLUP);
  pinMode(PIN_MODE_GND, OUTPUT);
  digitalWrite(PIN_MODE_GND, LOW); // Virtual GND for Mode button
  pinMode(PIN_PWM_GND, OUTPUT);
  digitalWrite(PIN_PWM_GND, LOW);  // Virtual GND for DAC logic ground
  pinMode(PIN_REL1_GND, OUTPUT);
  digitalWrite(PIN_REL1_GND, LOW);
  pinMode(PIN_REL2_GND, OUTPUT);
  digitalWrite(PIN_REL2_GND, LOW);

  currentMode = EEPROM.read(EEPROM_ADDR_MODE);
  if (currentMode > 4)
    currentMode = 0;
  currentPwmMin = currentMode * 61;

  activateFailsafe();
  if (display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    display.setRotation(1);
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(17, 15);
    display.print(F("POWER"));
    display.setTextSize(2);
    display.setCursor(20, 30);
    display.print(F("ON"));
    display.display();
  }
  LoRa.setPins(8, 4, 7);
  if (LoRa.begin(433E6))
    LoRa.setSyncWord(0xF1);
  currentState = SystemState::SEARCHING;
  lastReception = millis();
  lastActivity = millis();
  
  // Setup complete
  wdt_enable(WDTO_1S); // Capa 2: Watchdog de 1 segundo para recuperacion EMI
}

// === 1. POWER MANAGEMENT ===
void sleepSystem() {
  wdt_disable(); // Desactivar Watchdog antes de dormir
  outputsEnabled = false;
  activateFailsafe();
  digitalWrite(PIN_LED, LOW);
  display.ssd1306_command(SSD1306_DISPLAYOFF);
  LoRa.sleep();

  // Disable USB controller completely before sleep (notifies host PC of clean disconnect)
  #if defined(USBCON)
  USBCON = 0;
  #endif

  ADCSRA &= ~(1 << ADEN); // Disable ADC safely by clearing only the Enable bit
  powerState = false;
  while (digitalRead(PIN_MODE) == LOW)
    delay(10);
}

void wakeSystem() {
  powerState = true;
  ADCSRA |= (1 << ADEN); // Re-enable ADC preserving original Arduino prescaler
  sT = 0; // Reset battery smoothing to snap immediately to real values on wake
  sR = 0;

  // Re-initialize USB controller cleanly from scratch
  #if defined(USBCON)
  USBDevice.attach();
  #endif

  digitalWrite(PIN_LED, HIGH);
  display.ssd1306_command(SSD1306_DISPLAYON);
  currentMode = EEPROM.read(EEPROM_ADDR_MODE);
  if (currentMode > 4)
    currentMode = 0;
  currentPwmMin = currentMode * 61;
  currentState = SystemState::SEARCHING;
  isStandby = false; // Reset standby flag to boot in active searching state
  lastReception = millis();
  lastActivity = millis();

  // Wake complete
  wdt_enable(WDTO_1S); // Reactivar Watchdog al despertar
}

// === 2. MAIN LOOP ===
void loop() {
  wdt_reset(); // Alimentar al Watchdog en cada ciclo

  if (!powerState) {
    wdt_disable(); // Desactivar WDT durante el sueño profundo
    ADCSRA &= ~(1 << ADEN); // Ensure ADC is OFF (preserving prescaler)
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
    sleep_enable();
    attachInterrupt(digitalPinToInterrupt(PIN_MODE), []() {}, LOW);
    sei();
    sleep_cpu();
    sleep_disable();
    ADCSRA |= (1 << ADEN); // Re-enable ADC preserving prescaler
    detachInterrupt(digitalPinToInterrupt(PIN_MODE));
    if (digitalRead(PIN_MODE) == LOW) {
      wakeSystem();
      while (digitalRead(PIN_MODE) == LOW)
        delay(10);
    }
    return;
  }

  // Main button check
  bool btn = (digitalRead(PIN_MODE) == LOW);
  static bool btnL = false;
  static unsigned long btnS = 0;
  if (btn && !btnL) {
    btnS = millis();
  }
  if (btn && (millis() - btnS > 2000)) {
    sleepSystem();
    btnL = false;
    return;
  }
  if (!btn && btnL && (millis() - btnS < 600)) {
    currentMode = (currentMode + 1) % 5;
    EEPROM.update(EEPROM_ADDR_MODE, currentMode);
    currentPwmMin = currentMode * 61;
  }
  btnL = btn;

  // Optimized Payload parsing & Reconstructed compatible struct values
  if (LoRa.parsePacket() == sizeof(PedalDataPayload)) {
    PedalDataPayload rxPayload;
    LoRa.readBytes((uint8_t *)&rxPayload, sizeof(PedalDataPayload));
    
    // Decode switch closed flag
    myPedal.switchClosed = (rxPayload.flags & 0x01) ? true : false;
    
    // Decode distance travel mapping
    if (rxPayload.laserM == 255) {
      myPedal.laserDist = 999; // Standby
    } else {
      // Reconstruct millimeter value: map 0..254 back to 10..150mm
      myPedal.laserDist = map(rxPayload.laserM, 0, 254, 10, 150);
    }
    
    // Reconstruct raw ADC battery voltage (divided by 4 on TX)
    myPedal.batV = (uint16_t)rxPayload.batRaw * 4;

    lastReception = millis();
    lastActivity = millis();
    lastRSSI = LoRa.packetRssi();
    isStandby = (myPedal.laserDist == 999);
    systemLocked = isStandby;
    outputsEnabled = !isStandby;
    if (isStandby)
      activateFailsafe();
    currentState = isStandby ? SystemState::STANDBY : SystemState::CONNECTED;
  }

  // Keep system awake if button is pressed (USB does not prevent sleep)
  if (btn) {
    lastActivity = millis();
  }

  // Hot-plug USB detection for IDE
  static bool lastVbusRx = false;
  bool currentVbusRx = (USBSTA & (1 << VBUS));
  if (currentVbusRx && !lastVbusRx) {
    USBDevice.detach();
    delay(500);
    USBDevice.attach();
  }
  lastVbusRx = currentVbusRx;

  if (millis() - lastActivity > (DEEP_SLEEP_MIN * 60000UL)) {
    sleepSystem();
    return;
  }

  unsigned long dt = millis() - lastReception;

  // Dynamic timeout: 1s for Active, 10s for Standby
  bool timedOut = (!isStandby && dt > FAILSAFE_LIMIT) || (isStandby && dt > 10000);

  if (timedOut) {
    activateFailsafe();
    outputsEnabled = false;
    systemLocked = true;
    // We preserve isStandby's state (retaining visual Standby parpadeo if we timed out from Standby)
    currentState = SystemState::DISCONNECTED;
  } else if (!isStandby && dt > 200) {
    currentState = SystemState::HOLDING;
  }

  if (outputsEnabled) {
    currentPWM = map(constrain(myPedal.laserDist, PEDAL_DOWN_MM, PEDAL_UP_MM),
                     PEDAL_UP_MM, PEDAL_DOWN_MM, currentPwmMin, PWM_MAX_VAL);
    pwmOut.writePWM(currentPWM);
    rel1.writeDigital(myPedal.switchClosed);
    rel2.writeDigital(myPedal.switchClosed);
  }

  readBatteries();
  updateDisplay();

  sysBuzzer.update();

  // LED status indicator (Blinks in Standby, solid ON when active)
  if (isStandby) {
    digitalWrite(PIN_LED, (millis() % 1500 < 1000) ? HIGH : LOW);
  } else {
    digitalWrite(PIN_LED, HIGH);
  }

  // Battery Alarm Logic (Dual-layer monitoring in millivolts)
  static bool alarm10Triggered = false; // Level 1: 3.2V (10% display) -> 3 Buzzes
  static bool alarm5Triggered = false;  // Level 2: 3.1V (0% display) -> 5 Buzzes
  static bool alarm3Triggered = false;  // Level 3: 3.0V (Critical shutdown warning) -> 10 Buzzes

  bool txActive = (currentState == SystemState::CONNECTED || 
                   currentState == SystemState::STANDBY || 
                   currentState == SystemState::HOLDING);
  int lowestBatMV = (txActive && sT > 0)
                        ? min((int)sT, (int)sR)
                        : (int)sR;

  if (lowestBatMV <= 3000) {
    if (!alarm3Triggered) {
      sysBuzzer.trigger(10);
      alarm3Triggered = true;
      alarm5Triggered = true;
      alarm10Triggered = true;
    }
  } else if (lowestBatMV <= 3100) {
    if (!alarm5Triggered) {
      sysBuzzer.trigger(5);
      alarm5Triggered = true;
      alarm10Triggered = true;
    }
    if (lowestBatMV > 3020) {
      alarm3Triggered = false; // Hysteresis to re-enable Level 3
    }
  } else if (lowestBatMV <= 3200) {
    if (!alarm10Triggered) {
      sysBuzzer.trigger(3);
      alarm10Triggered = true;
    }
    if (lowestBatMV > 3120) {
      alarm5Triggered = false; // Hysteresis to re-enable Level 2
    }
  } else {
    if (lowestBatMV > 3220) {
      alarm10Triggered = false; // Hysteresis to re-enable Level 1
      alarm5Triggered = false;
      alarm3Triggered = false;
    }
  }
}

// === 3. HARDWARE CONTROL ===
void readBatteries() {
  static unsigned long lastBatRead = 0;
  if (millis() - lastBatRead < 500)
    return;
  lastBatRead = millis();

  ADMUX = _BV(REFS0) | _BV(MUX4) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  ADCSRA |= _BV(ADSC);
  while (bit_is_set(ADCSRA, ADSC))
    ;
  analogRead(PIN_BAT);
  delay(5); // Electrical quiet-period: prevents SPI/I2C noise from corrupting
            // high-impedance ADC sample capacitor
  localBatV = (uint16_t)(((uint32_t)analogRead(PIN_BAT) * RX_BATTERY_CALIBRATION) / 1000UL);
}

void activateFailsafe() {
  currentPWM = 0;
  pwmOut.writePWM(0);
  rel1.writeDigital(false);
  rel2.writeDigital(false);
}

// === 4. UI & DISPLAY ===
void updateDisplay() {
  static unsigned long lastD = 0;
  static bool lastStandbyState = false;

  // Atenuación automática del OLED en Standby
  if (isStandby != lastStandbyState) {
    if (isStandby) {
      display.ssd1306_command(SSD1306_SETCONTRAST);
      display.ssd1306_command(1); // Brillo mínimo
    } else {
      display.ssd1306_command(SSD1306_SETCONTRAST);
      display.ssd1306_command(255); // Brillo máximo
    }
    lastStandbyState = isStandby;
  }

  if (millis() - lastD < 50)
    return;
  lastD = millis();
  display.clearDisplay();
  int w = 62, h = 28, x_b = 1, half = 31, radius = 3;
  int ys[] = {2, 34, 66, 98};
  for (int y : ys) {
    display.drawRoundRect(x_b, y, w, h, radius, SSD1306_WHITE);
    display.drawFastVLine(x_b + half, y, h, SSD1306_WHITE);
  }

  display.drawBitmap(x_b + half / 2 - 9, 2 + h / 2 - 9, flash_bitmap, 18, 18, SSD1306_WHITE);
  int vP = (outputsEnabled) ? currentPWM : currentPwmMin;
  int pctP = (vP >= 244) ? 100 : map(vP, 0, PWM_MAX_VAL, 0, 100);
  if (myPedal.switchClosed && outputsEnabled) {
    display.fillRect(x_b + half + 1, 3, half - 2, h - 2, SSD1306_WHITE);
    display.setTextColor(SSD1306_BLACK);
  } else
    display.setTextColor(SSD1306_WHITE);
  int tx = (pctP < 10) ? 10 : (pctP < 100 ? 7 : 4);
  display.setTextSize(1, 2);
  display.setCursor(x_b + half + tx, 4 + (h - 16) / 2);
  display.print(pctP);
  display.print("%");

  if (!isStandby || (millis() % 1500 < 1000))
    display.drawBitmap(x_b + (half - 15) / 2, 34 + (h - 17) / 2, antenna_bitmap,
                       15, 17, SSD1306_WHITE);
  if (currentState == SystemState::DISCONNECTED) {
    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(1);
    display.setCursor(x_b + half + 7, 34 + (h - 8) / 2 + 1);
    display.print(F("---"));
  } else {
    int sP = constrain(map(lastRSSI, -115, -75, 0, 100), 0, 100);
    int bA = map(sP, 0, 100, 0, 5);
    if (sP > 0 && bA == 0)
      bA = 1;
    for (int i = 0; i < 5; i++)
      if (i < bA)
        display.fillRect(x_b + half + 4 + i * 5,
                         34 + h - 4 - map(i, 0, 4, 4, 20), 3,
                         map(i, 0, 4, 4, 20), SSD1306_WHITE);
  }

  if (currentState != SystemState::DISCONNECTED) {
    uint16_t tV = (uint16_t)(((uint32_t)myPedal.batV * TX_BATTERY_CALIBRATION) / 1000UL);
    if (sT == 0)
      sT = tV;
    sT = 0.02 * tV + 0.98 * sT;
  }
  if (sR == 0)
    sR = localBatV;
  sR = 0.02 * localBatV + 0.98 * sR;

  auto drawBat = [&](int y, const char *L, float val, bool disc) {
    bool isTX = (strcmp(L, "TX") == 0);
    int icon_x = x_b + (half - 26) / 2, icon_y = y + (h - 17) / 2 + 1;

    if (isTX)
      display.drawBitmap(icon_x, icon_y, icon_TX, 26, 17, SSD1306_WHITE);
    else
      display.drawBitmap(icon_x, icon_y, icon_RX, 26, 17, SSD1306_WHITE);
    if (disc) {
      display.setTextColor(SSD1306_WHITE);
      display.setTextSize(1);
      display.setCursor(x_b + half + 7, y + (h - 8) / 2 + 1);
      display.print(F("---"));
    } else {
      int p = map(constrain((int)val, 3100, 4100), 3100, 4100, 0, 100);
      int b = 0;
      if (p >= 80)
        b = 5;
      else if (p >= 60)
        b = 4;
      else if (p >= 40)
        b = 3;
      else if (p >= 20)
        b = 2;
      else if (p > 0)
        b = 1;
      for (int i = 0; i < 5; i++)
        if (i < b && !(p <= 10 && i == 0 && (millis() / 500 % 2)))
          display.fillRect(x_b + half + 4 + i * 5, y + 4, 3, h - 8, SSD1306_WHITE);
    }
  };
  drawBat(66, "TX", sT, currentState == SystemState::DISCONNECTED);
  drawBat(98, "RX", sR, false);
  display.display();
}
