# Migration Strategy: LoRa32u4 to ESP32-S3

This document outlines the technical factors and advantages of migrating the Wireless Welder Pedal ecosystem from the ATmega32u4 architecture to the ESP32-S3 (Heltec V3 / LILYGO) platform.

## 1. Executive Summary
The migration to ESP32-S3 is recommended to future-proof the project, reduce costs, and enable advanced features such as Mobile App connectivity and G-Code integration. Unlike the legacy system which used the same LoRa32u4 board for both units, the new architecture uses specialized hardware: the **Stick Lite V3** (optimized for low power in the pedal) and the **WiFi LoRa 32 V3** (equipped with OLED and WiFi for the receiver gateway).

## 2. Technical Comparison

| Feature | LoRa32u4 (Legacy) | ESP32-S3 (New) |
| :--- | :--- | :--- |
| **Model (Transmitter)**| LoRa32u4 RA-02 | Heltec Stick Lite V3 |
| **Model (Receiver)** | LoRa32u4 RA-02 | Heltec WiFi LoRa 32 V3|
| **Processor** | 8-bit AVR (8MHz) | 32-bit Dual-Core (240MHz) |
| **RAM / Flash** | 2.5KB / 32KB | 512KB / 8MB+ |
| **Connectivity** | LoRa 433MHz only | LoRa 433MHz + WiFi + BLE |
| **Radio Chip** | SX1278 | SX1262 (Lower power, more range) |
| **Deep Sleep** | ~100µA (Hardware dependent) | ~10µA - 50µA (S3 optimized) |
| **USB Connector** | Micro-USB | USB Type-C |
| **Battery Connector** | JST PH 2.0mm | SH 1.25mm |
| **Dimensions (L x W)**| 51 x 23 mm | 58x22mm (Stick) / 50x25mm (V3) |

## 3. Power Management for the Pedal (Transmitter)
One of the primary concerns is maintaining the long battery life of the pedal. The ESP32-S3 can achieve comparable or better performance through the following strategies:

*   **Modular Shutdown:** WiFi and Bluetooth can be explicitly turned off via software (`WiFi.mode(WIFI_OFF)` and `btStop()`) when the pedal is in active manual mode.
*   **Vext Control & Sensor Power:** While the legacy 32u4 uses a standard GPIO (Pin 5) to toggle sensor power, the ESP32 **Vext** port provides professional-grade power management:
    *   **Current Capacity:** Standard GPIOs are limited to ~20-40mA, which can be insufficient for peak ToF sensor demands. Vext is backed by a MOSFET and can handle 300-500mA, ensuring stable operation.
    *   **Hardware Isolation:** Vext physically disconnects the power rail, preventing parasitic current leakage that can occur through data lines (SDA/SCL) in GPIO-based setups during sleep.
    *   **Signal Stability:** The Vext rail provides cleaner power than a CPU I/O pin, reducing optical noise in precision measurements.
*   **Modern Deep Sleep:** The ESP32-S3 RTC (Real-Time Clock) allows for instant wake-up from GPIO interrupts (pedal movement) with extremely low current draw.

## 4. Radio Evolution: SX1278 vs. SX1262
While both chips are compatible at the protocol level (433MHz), the **SX1262** included in most ESP32 V3 boards offers:
*   **Reduced RX Current:** Uses approx. 40% less power during reception.
*   **Higher Link Budget:** Slightly better range and obstacle penetration.
*   **Better Safety:** Improved handling of transmission failures.

## 5. Recommended Hardware Models
To ensure full compatibility and use of the features described above, the following models are specified:
*   **Transmitter (Pedal):** **Heltec Wireless Stick Lite V3 (SX1262)**. Chosen for its slim form factor, low power consumption (No OLED), and integrated Vext control.
*   **Receiver:** **Heltec WiFi LoRa 32 V3 (SX1262)**. Chosen for its integrated OLED display, S3 processing power, and full WiFi/BLE gateway capabilities.

## 6. Implementation Notes
*   **Vext Control (Stick Lite V3):** The Vext power rail on the Wireless Stick Lite V3 is controlled via **GPIO 36**. 
    *   `digitalWrite(36, LOW)` enables power to the sensor.
    *   `digitalWrite(36, HIGH)` physically cuts power for ultra-low consumption.
*   **Pins:** Pin mapping will change; the ESP32 has many more GPIOs but requires careful selection of "strapping pins."
*   **Battery Connector Change (CRITICAL):** 
    *   **Legacy (LoRa32u4):** Uses a **JST PH 2.0mm** (2.0mm pitch).
    *   **New (ESP32 V3):** Uses a **SH 1.25mm** (1.25mm pitch).
    *   **Warning:** Most Heltec boards use a specific polarity (looking at the connector with pins facing you, Red/+ is typically on the right, but this varies by clone). **Always use a multimeter** to verify the board's polarity before plugging in a battery to avoid blowing the charging IC.
*   **Libraries:** Transition from `LoRa.h` to more modern libraries like `RadioLib` or the Heltec-specific libraries is recommended for full SX1262 support.
*   **Logic:** The core safety logic (Failsafes, Median Filters) will be ported as a new "Core" library shared between both units.
