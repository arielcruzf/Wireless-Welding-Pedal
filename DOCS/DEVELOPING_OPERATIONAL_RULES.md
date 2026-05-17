# Developing Operational Rules

This document outlines the mandatory operational and development rules that the AI coding assistant and developers must strictly follow when working on the **Wireless Welder Pedal** project.

---

## 1. Code & Comment Language Policy
*   **Mandatory Language:** All code symbols (variable names, function names, classes, structs, enums) and **all comments inside source code files** (`.ino`, `.h`, `.cpp`, etc.) **MUST** be written in **English**.
*   **Aesthetic & Communication:** The developer pair-programming dialogue can be bilingual (Spanish/English) as requested by the user, but the source code itself must remain 100% clean, standardized, and professional in English.

---

## 2. Git & GitHub Push Policy
*   **Strict Control:** **DO NOT** push any code or commits to GitHub automatically.
*   **Explicit Approval:** The assistant is allowed to perform local commits (`git commit`) to keep track of changes, but **MUST NOT** execute `git push` to the remote repository (e.g., `origin main`) unless the user explicitly requests it (e.g., "Súbelo a GitHub", "Push origin main", etc.).

---

## 3. Dynamic AI Update ("CHAT IA" Protocol)
*   **Knowledge Synchronization:** When the user or assistant makes significant changes to the firmware architecture or logic, the assistant must update the context logs, system blueprint, or project documents to ensure the RAG (Retrieval-Augmented Generation) knowledge base is synchronized.
*   **State Alignment:** Any changes that alter pinouts, timers, calibration multipliers, or core states must be logged immediately so the next AI pair-programming agent is perfectly aligned.

---

## 4. Hardware & Firmware Coding Directives
To ensure maximum safety, reliability, and zero latency:
1.  **Non-Blocking Logic:** Never use `delay()` in the main loops. Use non-blocking timers (`millis()`) for state transitions, display updates, and buzzer alerts. Exceptions are allowed only during the `setup()` boot phase (e.g., USB grace period) or physical quiet-periods for the ADC.
2.  **SRAM Memory Preservation:** Every literal string printed to the Serial Monitor **MUST** be encapsulated in the `F()` macro (e.g., `Serial.println(F("Hello"))`) to prevent SRAM memory exhaustion.
3.  **Non-Blocking Diagnostics:** All telemetry and diagnostic logs in the Serial Monitor must be wrapped in `if (Serial)` check, so they execute instantly and bypass printing overhead entirely when no USB Serial Monitor is connected.
4.  **Hardware-Driven Calibration:** Battery calibration multipliers (`RX_CALIBRATION`, `TX_CALIBRATION`) should be adjusted using the live suggestions generated automatically in the Serial Monitor telemetry block.
5.  **Relay Failsafe Enforcement:** The welding output relays (`PIN_REL1`, `PIN_REL2`) and the PWM signal must be immediately de-energized/locked to `0` when a signal loss (LoRa timeout) or Standby state is detected.
