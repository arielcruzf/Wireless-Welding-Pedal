# Smart Receiver Expansion - Wireless Welder Pedal

This document defines the approach for evolving the current receiver into a **Smart Gateway**. The goal is to provide the system with bidirectional connectivity with mobile devices and computers, while maintaining the robustness of the 433MHz LoRa link.

## 1. Concept: The Receiver as a "Bridge"
The receiver stops being an isolated terminal to become a communication node. Its main function is to act as an intermediary between the physical pedal (Critical Hardware) and modern control interfaces (Software).

### Bidirectional Data Flow:
1.  **Uplink (Monitoring):** Pedal (LoRa 433MHz) → Receiver → Mobile/PC (BLE/WiFi).
    *   Real-time telemetry transmission: Pedal pressure, battery status (TX/RX), signal quality (RSSI).
    *   Usability data logging (Datalogging).
2.  **Downlink (Control):** Mobile/PC (BLE/WiFi) → Receiver → Welder (PWM/Relays).
    *   Remote activation commands (Trigger) and power regulation.
    *   Dynamic parameter configuration (Sensitivity curves, PWM limits).

---

## 2. Hardware: ESP32 LoRa Module Integration
To achieve this connectivity without sacrificing the stability of 433MHz, the transition to the **ESP32-S3 LoRa (433MHz)** module is recommended.

### Advantages of the change:
*   **Dual-Core:** Separation of critical tasks (LoRa) from networking tasks (WiFi/Bluetooth).
*   **Bluetooth Low Energy (BLE):** Native connection with mobile Apps with minimal latency.
*   **WiFi:** Connectivity with computers and industrial networks (MQTT, HTTP, WebSockets).
*   **Compatibility:** Maintains the SX1278 chip, ensuring full communication with the current transmitter.

---

## 3. Expansion Objectives

### A. Telemetry and Mobile APP
*   **Data Logging:** Visualization of "Arc Time" (Duty Cycle) for productivity control.
*   **Diagnostics:** Low battery or signal loss alerts directly on the mobile phone.
*   **Presets:** Creation and loading of customized welding profiles from the App.

### B. Industrial Integration (G-Code & CNC)
*   **Virtual Trigger:** Ability to command the welder's activation from a plasma cutter or CNC machine.
*   **G-Code Parser:** The receiver will be able to interpret commands like `M03` (Turn ON), `M05` (Turn OFF), and `S[value]` (Power) received via Serial or WiFi.

### C. Robotics
*   **Robot Arm Control:** Simplified interface for a robot to control trigger and power wirelessly, facilitating integration into automated welding cells.

---

## 4. Safety and Control Priority
The system will implement a safety hierarchy to prevent accidents:
1.  **LoRa Failsafe:** If the connection with the physical pedal is lost while operating manually, the system cuts the output instantly.
2.  **Physical Priority:** The physical pedal will always have the ability to "override" or stop a command sent via Bluetooth/WiFi.
3.  **Network Watchdog:** If an activation command comes via WiFi/BT, the receiver will require a constant "heartbeat"; if the wireless connection fails, welding stops.

---

## 5. Roadmap
1.  **Phase 1:** Migration of the current receiver code to the ESP32 environment (Maintaining basic functionality).
2.  **Phase 2:** Implementation of BLE service for sending telemetry to serial monitor/mobile.
3.  **Phase 3:** Development of a basic command parser for remote control (Trigger/Power Mode).
4.  **Phase 4:** Creation of a minimal web interface (Web Dashboard) hosted on the receiver for control from a PC.
