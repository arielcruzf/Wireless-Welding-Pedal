# TECHNICAL SPECIFICATIONS: FIRMWARE AND LIBRARIES

This document centralizes technical findings regarding project programming and libraries for the **Wireless Welder Pedal** AI knowledge base.

---

## 🚀 1. FLASHING PROCEDURE

To upload code to the **LoRa32u4 RA-02 (Transmitter and Receiver)** modules:

*   **Board Profile:** In the Arduino IDE, select **"Arduino Leonardo"**. The ATmega32u4 chip is 100% compatible with this profile.
*   **Bootloader Management ("Double-Reset" Technique):** The ATmega32u4 handles USB internally. If the code crashes or the port disappears:
    1. Click **"Upload"** in the Arduino IDE.
    2. Wait for the IDE to change from "Compiling" to **"Uploading..."**.
    3. At that exact moment, press the physical **RESET button twice quickly**. This opens an 8-second window for the IDE to catch the bootloader port.
*   **Troubleshooting on Mac (Sonoma/Ventura):**
    *   If the port does not appear, run in Terminal: `sudo killall usbd`.
    *   On Macs with Apple Silicon, ensure that in *Privacy & Security*, the *"Allow accessories to connect"* option is set to **Always**.
*   **Serial Communication:** Use `Serial.begin(115200)`. Being a Leonardo, the serial port does not reset when opening the monitor unless `while(!Serial)` is used.

---

## 📚 2. LIBRARY MANAGEMENT

### VL53L4CD Laser Sensor (CRITICAL)
A severe hardware incompatibility was discovered between the ATmega32u4 chip and the official ST library:
*   **Mandatory Solution (Pololu Library):** It is mandatory to use the **"VL53L4CD" library by Pololu** ([GitHub: pololu/vl53l4cd-arduino](https://github.com/pololu/vl53l4cd-arduino)). This library was specifically designed for limited microcontrollers:
    1.  It automatically splits I2C transmissions into chunks smaller than 32 bytes.
    2.  It implements software timeouts (`sensor.setTimeout()`) that ensure the code **never** freezes, even if the laser is physically destroyed or its wires are cut.
*   **Final Action:** All `XSHUT` pin control code was removed from the transmitter. The sensor now operates freely, commanded only by I2C read requests.

## PHASE 5: Final Calibration and Advanced Filtering (v10.3)
*   **Range Adjustment:** The working range was standardized to **15mm (100%) to 50mm (0%)**.
*   **Dual Filter (Median + EMA):**
    *   **Median Filter (5 samples):** Implemented to eliminate ±3mm reading noise (jitter). This filter ignores spikes and maintains the central value.
    *   **EMA Filter (0.4):** Applied over the median value to ensure smooth transitions.
*   **Standby Mode:** Automatic LoRa radio disconnection was added after 5 minutes of inactivity, with a **flashing Hourglass icon (100ms)** on the receiver to indicate this state.
*   **Latency Optimization:** The sensor timing budget was adjusted to 50ms and the transmission loop to 30ms.
*   **Mandatory Installation:** 
    *   In the Arduino IDE, navigate to `Sketch -> Include Library -> Manage Libraries...`
    *   Search for `VL53L4CD` and install the version authored by **Pololu**.
*   **API Note (Pololu):** Initialization is performed with `sensor.init()`. Reading is done with `sensor.read()`, which automatically retrieves the distance and clears the sensor's internal interrupt, drastically simplifying the code.

### LoRa Radio (SX1278)
*   **Library:** LoRa by Sandeep Mistry.
*   **Frequency:** 433E6 (433MHz).
*   **Internal Pins (Hard-wired):**
    *   **NSS/CS:** Pin 8
    *   **RST:** Pin 4
    *   **DIO0 (IRQ):** Pin 7
*   **Initialization:** `LoRa.setPins(8, 4, 7);`

---

## ⚠️ 3. SAFETY AND HARDWARE (CRITICAL NOTES)

*   **Antenna:** **NEVER** power or flash the board without the 433MHz antenna connected. Reflected power can instantly burn the SX1278 chip.
*   **Power Switch (M10 LED Push Button):** The master switch connects to the **EN** and **GND** pins. Closing the circuit (EN to GND) turns off the board's regulator, safely deactivating the system without physically disconnecting the battery.

### Calibration Parameters (v10.3)
- **Idle Distance:** 50mm (0% PWM).
- **Full Press Distance:** 15mm (100% PWM / 244 DAC).
- **Sampling Frequency:** 30ms.
- **Filtering:** 5-sample Median + EMA (0.4).

### Power Management
- **Laser Standby:** The laser enters power-saving mode when the trigger is not pressed.
- **Radio Standby:** The LoRa module turns off after 5 minutes of total inactivity.
- **Re-connection Protocol:** The transmitter sends the code `999` before sleeping. It wakes up instantly (<30ms) upon detecting a press on Pin 11.

*   **I2C Pinout:**
    *   **SDA:** Pin 2
    *   **SCK / SCL:** Pin 3
*   **GND Isolation:** The receiver's MOSFET #2 is used to disconnect the DAC module's GND, guaranteeing 0.00V at rest.

---

*Document generated for RAG training and technical project support.*
