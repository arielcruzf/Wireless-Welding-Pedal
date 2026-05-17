# Wireless Welder Pedal V1.0 - System Logic & Architecture

This document defines the technical knowledge base (AI Knowledge Base) for the "Wireless Welder Pedal V1.0" project.

## 1. General Project Function
The project consists of a professional-grade wireless TIG welding pedal. Instead of using a traditional mechanical potentiometer (which suffers from friction and dust wear), it uses a **Time-of-Flight (ToF) Laser Sensor - VL53L4CD** to measure the pedal depression depth with millimeter precision without physical contact. Communication is handled via **LoRa (433MHz)** radio frequency, ensuring extreme range and resistance to electromagnetic interference typical in welding workshops. 

The system is divided into two main parts: the **Transmitter (Pedal)** and the **Receiver (Base unit connected to the welder)**.

---

## 2. Technical Definition: Transmitter (TX)
**(Code location: `SOFTWARE/transmitter/transmitter.ino`)**

The transmitter module mechanically reads the user's state and telemetry, sending it continuously to the receiver. Its special features include:

*   **Smart ToF Laser Management:** To maximize battery life, the laser operates in three distinct states:
    1.  **OFF (Hardware Power-Down):** Physical power is cut (Pin 5 LOW) and I2C lines are isolated during Deep Sleep to eliminate phantom power and prevent hardware lockups.
    2.  **IDLE (Software Stop):** The sensor is powered but not emitting light. This is the default state when the pedal is released.
    3.  **ACTIVE (Continuous Ranging):** The laser is measuring distance. This occurs *only* while the trigger is actively pressed, turning off instantly when released.
*   **Deep Sleep Wake-Up Architecture:** The system employs a hardware-safe Pin Change Interrupt (`PCINT0_vect`) to wake the processor from deep sleep. This prevents the "Zombie Trigger Bug" where unhandled interrupts would cause the AVR to soft-reset, corrupting the I2C bus and LoRa state. Upon waking up, the system performs a fully controlled re-initialization of the sensor, I2C bus, and radio to guarantee an instant, lag-free connection.
*   **Compound Stabilization Filter:** The optical laser reading passes through two real-time mathematical filters:
    1.  *Median Filter (3-value buffer):* Discards absurd peaks caused by erratic reflections.
    2.  *Exponential Smoothing:* Prevents "jitter" or shaking in the welder's control, delivering a perfectly smooth acceleration curve. If there's a sudden change (>3mm), the filter reacts instantly to avoid latency.
*   **Advanced Power Management (Deep Sleep):** After 10 minutes of inactivity, the system enters `SLEEP_MODE_PWR_DOWN` mode. Power consumption drops to microamps. To wake the system, the user must press the "MODE" button.

---

## 3. Technical Definition: Receiver (RX)
**(Code location: `SOFTWARE/receiver/receiver.ino`)**

The receiver module takes data packets via LoRa, evaluates safety, updates the visual interface, and modulates the physical signal (PWM and Relays) to the welding machine.

*   **5 Power Range Modes (PWM):** This is one of the flagship features. It allows the user to adjust the pedal's sensitivity by limiting the starting point of the power. It is navigated by pressing the front button and the state is saved in non-volatile memory (EEPROM):
    *   **Mode 0:** 0% to 100% (Full range).
    *   **Mode 1:** 25% to 100% (Quick start).
    *   **Mode 2:** 50% to 100%.
    *   **Mode 3:** 75% to 100%.
    *   **Mode 4:** 100% Fixed (Acts as a simple ON/OFF trigger).
*   **Safety Failsafe (Watchdog):** A critical function to prevent serious accidents. If the receiver stops receiving data from the pedal for more than `FAILSAFE_LIMIT` (1000 milliseconds), it assumes the link is broken and instantly cuts the PWM output and relays.
*   **Professional User Interface (OLED UI):** The vertical screen uses an optimized borderless design, drawing components directly from flash memory matrices (`PROGMEM`). It displays in real-time:
    *   Output PWM percentage.
    *   Antenna signal quality (RSSI bar).
    *   Pedal battery level with the special **PBT** (Pedal Battery) icon.
    *   Local receiver battery level with the **RBT** (Receiver Battery) icon.
*   **Memory Stability:** To guarantee uninterrupted "Up-Time" (ensuring the system never crashes while welding), the use of dynamic `String` objects was completely eliminated, using lightweight text pointers (`const char*`) instead, along with memory fragmentation prevention logic in the main loop.

## 4. System States & Power Management
The system is designed to maximize battery life while maintaining high responsiveness and safety. It operates in three distinct software states:

### A. Active Mode (Full Power)
*   **Trigger:** System wake-up or recent user activity.
*   **Behavior:** 
    *   **LED:** Solid ON.
    *   **LoRa:** Continuous transmission at ~50Hz (every 20ms) for zero-latency control.
    *   **Laser:** Active only when the physical trigger is pressed.
    *   **Receiver Watchdog:** Strict 1-second failsafe. If a packet is missed for >1000ms, the receiver cuts power.

### B. Standby Mode (Smart Wait)
*   **Trigger:** 2 minutes of inactivity (no pedal movement or button presses).
*   **Behavior:** 
    *   **LED:** "Heartbeat" breathing pattern.
    *   **LoRa:** Enters a low-power "Sleep" state, waking up every 2 seconds to send a **Heartbeat Packet**.
    *   **Laser:** Completely disabled (Software Stop).
    *   **Heartbeat Logic ("Code 999"):** The transmitter sends a special packet with `laserDist = 999`. Upon receiving this code, the Receiver enters its own Standby state and switches its watchdog from 1 second to a **10-second timeout**, accommodating the 2-second heartbeat interval.
*   **Wake-up:** Instantaneous. Pressing the pedal trigger immediately reverts the system to Active Mode.

### C. Deep Sleep Mode (Ultra Low Power)
*   **Trigger:** 10-15 minutes of inactivity, manual shutdown (2s button hold), or critical low battery (<3.3V).
*   **Behavior:** 
    *   **Hardware:** Power to the laser sensor and peripherals is physically cut (Pin 5 LOW). LED is OFF.
    *   **CPU:** Enters `SLEEP_MODE_PWR_DOWN`, consuming less than 1mA.
    *   **Communication:** No data is sent.
*   **Wake-up:** Requires a physical interrupt (pressing the "MODE" button or the pedal trigger). The system performs a full hardware re-initialization upon waking.

---

## 5. USB Connectivity & Development Stability
To ensure professional-grade reliability during firmware updates and debugging, both units implement specific USB handling:
*   **USB Grace Period:** A 3-second delay at boot to ensure the operating system correctly enumerates the Serial port before the main application starts.
*   **Wake-Up Re-enumeration:** When waking from Deep Sleep, the units force a `detach/attach` cycle on the USB device, making the COM port reappear instantly in the IDE without a manual reset.
*   **Hot-Plug Detection (VBUS Monitoring):** The system continuously monitors the hardware `VBUS` state. If a USB cable is connected while the system is active, it automatically triggers a re-enumeration cycle. This allows the Arduino IDE to recognize the board instantly upon connection without requiring a physical reset button press.
*   **Power State Independence:** The power management logic (Standby and Deep Sleep) is **completely independent** of the USB connection. The device will follow its sleep timers even if plugged into a computer or charger. This ensures consistent power behavior and prevents the device from staying awake indefinitely just because it is connected to a power source.

---

## Conclusion
What makes this system so special is its **industrial-grade reliability built on accessible hardware**. The combination of bidirectional telemetry, safety against radio failures (failsafe), frictionless optical reading (ToF), and dynamic power consumption control makes it a vastly superior alternative to conventional wired mechanical pedals.
