#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <EEPROM.h>
#include <LoRa.h>
#include <SPI.h>
#include <Wire.h>
#include <avr/sleep.h>

/** PROJECT: Wireless Welder Pedal v1.0 POWER-MASTER | RECEIVER UNIT */

// =============================================================
//                    USER CONFIGURATION
// =============================================================
const long FAILSAFE_LIMIT = 1000;        // Max wait time for signal
const unsigned long DEEP_SLEEP_MIN = 2; // Minutes before Deep Sleep
const int PEDAL_UP_MM = 60;              // Pedal up distance
const int PEDAL_DOWN_MM = 17;            // Pedal down distance
const int PWM_MAX_VAL = 244;             // Max PWM output value
const uint32_t RX_BATTERY_CALIBRATION =
    17850UL; // Physical battery voltage calibration for Receiver
const uint32_t TX_BATTERY_CALIBRATION =
    17500UL; // Physical battery voltage calibration for Transmitter
// Note: To calibrate these values, simply connect a USB cable and open
// the Serial Monitor (115200 baud). The live telemetry console will
// automatically display the exact RX_BATTERY_CALIBRATION and
// TX_BATTERY_CALIBRATION values for you to copy and paste here.

// =============================================================
//                    HARDWARE PINOUT
// =============================================================
#define PIN_PWM 10
#define PIN_REL1 12
#define PIN_REL2 11
#define PIN_BAT A0
#define PIN_MODE 0
#define PIN_LED A9
#define PIN_BUZZER 6

#define EEPROM_ADDR_MODE 0
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// GLOBAL VARIABLES
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
struct __attribute__((packed)) PedalData {
  uint16_t laserDist;
  bool switchClosed;
  uint16_t batV;
};
PedalData myPedal;
uint16_t localBatV = 0;
int lastRSSI = 0, currentPWM = 0, currentPwmMin = 0;
uint16_t lastValidLaserDist = 60; // Última distancia láser registrada antes del apagado
uint16_t calMin = 999;             // Calibración dinámica: valor mínimo de distancia
uint16_t calMax = 0;               // Calibración dinámica: valor máximo de distancia
unsigned long lastReception = 0, lastActivity = 0;
float sT = 0,
      sR = 0; // Global battery smoothing variables to allow reset on wake
bool systemLocked = true, isStandby = false, outputsEnabled = false,
     powerState = true;
enum class SystemState { STARTING, SEARCHING, STANDBY, CONNECTED, DISCONNECTED, HOLDING };
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
  void begin() { pinMode(pin, OUTPUT); writeDigital(LOW); }
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
    if (buzzesRemaining > 0) return; // Don't override ongoing alarm
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
  delay(3000); // USB Grace Period for IDE recognition
  Serial.begin(115200);
  pinMode(PIN_LED, OUTPUT);
  digitalWrite(PIN_LED, HIGH);
  pwmOut.begin();
  rel1.begin();
  rel2.begin();
  sysBuzzer.begin();
  pinMode(PIN_MODE, INPUT_PULLUP);
  pinMode(5, OUTPUT);
  digitalWrite(5, LOW);

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
}

// === 1. POWER MANAGEMENT ===
// Mode 0: Start from 0% (Full Range).
// Mode 1: Start from 25% (Range 25-100%).
// Mode 2: Start from 50% (Range 50-100%).
// Mode 3: Start from 75% (Range 75-100%).
// Mode 4: Fixed at 100% (Trigger Mode).
void sleepSystem() {
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
                          // (preserves prescaler)
  powerState = false;
  while (digitalRead(PIN_MODE) == LOW)
    delay(10);
}

void wakeSystem() {
  powerState = true;
  ADCSRA |= (1 << ADEN); // Re-enable ADC preserving original Arduino prescaler
                         // (prevents 4MHz ADC clock bug)
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
  lastReception = millis();
  lastActivity = millis();
}

// === 2. MAIN LOOP ===
void loop() {
  if (!powerState) {
    ADCSRA &= ~(1 << ADEN); // Ensure ADC is OFF (preserving prescaler)
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
    sleep_enable();
    attachInterrupt(
        digitalPinToInterrupt(PIN_MODE), []() {}, LOW);
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

  if (LoRa.parsePacket() == sizeof(PedalData)) {
    LoRa.readBytes((uint8_t *)&myPedal, sizeof(PedalData));
    lastReception = millis();
    lastActivity = millis();
    lastRSSI = LoRa.packetRssi();
    isStandby = (myPedal.laserDist == 999);
    systemLocked = isStandby;
    outputsEnabled = !isStandby;
    if (isStandby)
      activateFailsafe();
    else if (myPedal.switchClosed && myPedal.laserDist != 150 && myPedal.laserDist != 999) {
      lastValidLaserDist = myPedal.laserDist;
      if (myPedal.laserDist < calMin) calMin = myPedal.laserDist;
      if (myPedal.laserDist > calMax) calMax = myPedal.laserDist;
    }
    currentState = isStandby ? SystemState::STANDBY : SystemState::CONNECTED;
  }

  // Keep system awake if button is pressed (USB does not prevent sleep)
  if (btn) {
    lastActivity = millis();
  }

  // Detección de conexión USB en caliente para el IDE
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
  bool timedOut =
      (!isStandby && dt > FAILSAFE_LIMIT) || (isStandby && dt > 10000);

  if (timedOut) {
    activateFailsafe();
    outputsEnabled = false;
    systemLocked = true;
    isStandby = false;
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

  // Battery Alarm Logic
  static bool alarm10Triggered = false;
  static bool alarm5Triggered = false;

  int pTX = (currentState != SystemState::DISCONNECTED) ? map(constrain((int)sT, 3100, 4100), 3100, 4100, 0, 100) : 100;
  int pRX = map(constrain((int)sR, 3100, 4100), 3100, 4100, 0, 100);
  int lowestBat = min(pTX, pRX);

  if (lowestBat <= 5) {
    if (!alarm5Triggered) {
      sysBuzzer.trigger(5);
      alarm5Triggered = true;
      alarm10Triggered = true;
    }
  } else if (lowestBat <= 10) {
    if (!alarm10Triggered) {
      sysBuzzer.trigger(3);
      alarm10Triggered = true;
    }
    if (lowestBat > 7) {
      alarm5Triggered = false;
    }
  } else {
    if (lowestBat > 12) {
      alarm10Triggered = false;
      alarm5Triggered = false;
    }
  }

  // --- TELEMETRY & CALIBRATION SERIAL LOGS ---
  if (Serial) {
    static unsigned long lastSerialPrint = 0;
    if (millis() - lastSerialPrint >= 500) {
      lastSerialPrint = millis();
      
      Serial.println(F("\n============================================================"));
      Serial.println(F("           WIRELESS WELDER PEDAL TELEMETRY & CALIBRATION"));
      Serial.println(F("============================================================"));
      
      // 1. Link Status
      Serial.print(F("[LINK STATUS]  "));
      if (currentState == SystemState::DISCONNECTED) {
        Serial.println(F("DISCONNECTED"));
        Serial.print(F("[LASER DIST]   Current: --- mm | Last Released (Rest): "));
        if (lastValidLaserDist == 150) Serial.println(F("--- mm"));
        else { Serial.print(lastValidLaserDist); Serial.println(F(" mm")); }
      } else {
        if (currentState == SystemState::CONNECTED) {
          Serial.print(F("CONNECTED (RSSI: "));
          Serial.print(lastRSSI);
          Serial.println(F(" dBm)"));
        } else if (currentState == SystemState::HOLDING) {
          Serial.println(F("HOLDING (interference warning)"));
        }
        
        // 3. Laser Distance
        Serial.print(F("[LASER DIST]   Current: "));
        if (myPedal.laserDist == 150) {
          Serial.print(F("---"));
        } else {
          Serial.print(myPedal.laserDist);
        }
        Serial.print(F(" mm | Last Released (Rest): "));
        Serial.print(lastValidLaserDist);
        Serial.println(F(" mm"));
      }
      
      // 3b. Dynamic Pedal Calibration Helper
      Serial.println(F("               PEDAL CALIBRATION: PRESS AND RELEASE THE PEDAL ALL THE WAY DOWN 3 TIMES"));
      Serial.print(F("               const int PEDAL_UP_MM = "));
      if (calMax == 0) Serial.println(F("---;"));
      else { Serial.print(calMax); Serial.println(F(";")); }
      Serial.print(F("               const int PEDAL_DOWN_MM = "));
      if (calMin == 999) Serial.println(F("---;"));
      else { Serial.print(calMin); Serial.println(F(";")); }
      
      Serial.println(F("------------------------------------------------------------"));
      
      // 4. Battery RX
      Serial.print(F("[BATTERY RX]   Raw ADC: "));
      int rxRaw = analogRead(PIN_BAT);
      Serial.print(rxRaw);
      Serial.print(F(" | Voltage: "));
      Serial.print((float)sR / 1000.0, 2);
      Serial.print(F(" V | Status: "));
      int rxPct = map(constrain((int)sR, 3100, 4100), 3100, 4100, 0, 100);
      int rxBars = 0;
      if (rxPct >= 80) rxBars = 5;
      else if (rxPct >= 60) rxBars = 4;
      else if (rxPct >= 40) rxBars = 3;
      else if (rxPct >= 20) rxBars = 2;
      else if (rxPct > 0) rxBars = 1;
      Serial.print(F("["));
      for (int i = 0; i < 5; i++) {
        if (i < rxBars) Serial.print(F("|"));
        else Serial.print(F("-"));
      }
      Serial.print(F("] "));
      Serial.print(rxBars);
      Serial.println(F("/5"));
      if (rxRaw > 0) {
        Serial.print(F("               >>> RX_BATTERY_CALIBRATION = "));
        Serial.print((4230UL * 1000UL) / rxRaw);
        Serial.println(F("UL"));
      }
      
      // 5. Battery TX
      Serial.print(F("[BATTERY TX]   "));
      if (currentState == SystemState::DISCONNECTED) {
        Serial.println(F("Raw ADC: --- | Voltage: ---- V | Status: [-----] 0/5"));
      } else {
        Serial.print(F("Raw ADC: "));
        Serial.print(myPedal.batV);
        Serial.print(F(" | Voltage: "));
        Serial.print((float)sT / 1000.0, 2);
        Serial.print(F(" V | Status: "));
        int txPct = map(constrain((int)sT, 3100, 4100), 3100, 4100, 0, 100);
        int txBars = 0;
        if (txPct >= 80) txBars = 5;
        else if (txPct >= 60) txBars = 4;
        else if (txPct >= 40) txBars = 3;
        else if (txPct >= 20) txBars = 2;
        else if (txPct > 0) txBars = 1;
        Serial.print(F("["));
        for (int i = 0; i < 5; i++) {
          if (i < txBars) Serial.print(F("|"));
          else Serial.print(F("-"));
        }
        Serial.print(F("] "));
        Serial.print(txBars);
        Serial.println(F("/5"));
        if (myPedal.batV > 0) {
          Serial.print(F("               >>> TX_BATTERY_CALIBRATION = "));
          Serial.print((4180UL * 1000UL) / myPedal.batV);
          Serial.println(F("UL"));
        }
      }
      
      Serial.println(F("============================================================"));
    }
  }
}

// === 3. HARDWARE CONTROL ===
void readBatteries() {
  static unsigned long lastBatRead = 0;
  if (millis() - lastBatRead < 500) return;
  lastBatRead = millis();

  ADMUX = _BV(REFS0) | _BV(MUX4) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  ADCSRA |= _BV(ADSC);
  while (bit_is_set(ADCSRA, ADSC))
    ;
  analogRead(PIN_BAT);
  delay(5); // Electrical quiet-period: prevents SPI/I2C noise from corrupting high-impedance ADC sample capacitor
  localBatV = (uint16_t)((analogRead(PIN_BAT) * RX_BATTERY_CALIBRATION) / 1000UL);
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

  display.drawBitmap(x_b + half / 2 - 9, 2 + h / 2 - 9, flash_bitmap, 18, 18,
                     SSD1306_WHITE);
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
    uint16_t tV =
        (uint16_t)(((uint32_t)myPedal.batV * TX_BATTERY_CALIBRATION) / 1000UL);
    if (sT == 0)
      sT = tV;
    sT = 0.02 * tV + 0.98 * sT;
  }
  if (sR == 0)
    sR = localBatV;
  sR = 0.02 * localBatV + 0.98 * sR;

  // --- CALIBRATION MODE (Uncomment to use with Multimeter) ---
  // Serial.print("RX Raw ADC: ");
  // Serial.print(analogRead(PIN_BAT));
  // Serial.print(" | RX mV: ");
  // Serial.print(sR);
  // Serial.print(" || TX Raw ADC: ");
  // Serial.print(myPedal.batV);
  // Serial.print(" | TX mV: ");
  // Serial.println(sT);
  // ----------------------------------------------------

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
      if (p >= 80) b = 5;
      else if (p >= 60) b = 4;
      else if (p >= 40) b = 3;
      else if (p >= 20) b = 2;
      else if (p > 0) b = 1;
      for (int i = 0; i < 5; i++)
        if (i < b && !(p <= 10 && i == 0 && (millis() / 500 % 2)))
          display.fillRect(x_b + half + 4 + i * 5, y + 4, 3, h - 8,
                           SSD1306_WHITE);
    }
  };
  drawBat(66, "TX", sT, currentState == SystemState::DISCONNECTED);
  drawBat(98, "RX", sR, false);
  display.display();
}
