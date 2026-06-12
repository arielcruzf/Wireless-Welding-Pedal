# Developing Operational Rules

This document outlines the mandatory operational and development rules that the AI coding assistant and developers must strictly follow when working on the **Wireless Welder Pedal** project.

---

## 1. Code & Comment Language Policy
*   **Mandatory Language:** All code symbols (variable names, function names, classes, structs, enums) and **all comments inside source code files** (`.ino`, `.h`, `.cpp`, etc.) **MUST** be written in **English**.
*   **Aesthetic & Communication:** The developer pair-programming dialogue can be bilingual (Spanish/English) as requested by the user, but the source code itself must remain 100% clean, standardized, and professional in English.

---

## 2. Git & GitHub Update Policy
*   **Absolute Strict Restriction:** **NEVER** run any Git commands (such as `git add`, `git commit`, `git push`, etc.) or modify the repository state, whether locally or on remote platforms like GitHub, **unless the user explicitly and literally commands you to do so** (e.g., "actualiza todo en git", "push git hub", etc.).
*   **Freeze by Default:** By default, all Git operations are completely frozen. The assistant must focus solely on local file modifications in the workspace and never perform git commits, pushes, or staging unless explicitly instructed.

---

## 3. Dynamic AI Update ("CHAT IA" Protocol)
*   **Knowledge Synchronization:** When the user or assistant makes significant changes to the firmware architecture or logic, the assistant must update the context logs, system blueprint, or project documents to ensure the RAG (Retrieval-Augmented Generation) knowledge base is synchronized.
*   **State Alignment:** Any changes that alter pinouts, timers, calibration multipliers, or core states must be logged immediately so the next AI pair-programming agent is perfectly aligned.

---

## 4. Firmware Versioning & Working Target Policy
*   **Target the Latest Validated Version:** When applying modifications, refactoring, migrating pins, or implementing new features, we **MUST** always target and apply changes to the **latest validated version** of the software in the active development directories (e.g.,`receiver_v1.1.ino` in the `FIRMWARE/receiver_v1.1/` folder, and `transmitter_v1.1.ino` in `FIRMWARE/transmitter_v1.1/`).
*   **Baseline Preservation:** Older versions (e.g., legacy `v1.0` under the `receiver` and `transmitter` directories) must remain untouched as stable reference baselines unless the user explicitly requests changes to be backported.

---

## 5. Hardware & Firmware Coding Directives
To ensure maximum safety, reliability, and zero latency:
1.  **Non-Blocking Logic:** Never use `delay()` in the main loops. Use non-blocking timers (`millis()`) for state transitions, display updates, and buzzer alerts. Exceptions are allowed only during the `setup()` boot phase (e.g., USB grace period) or physical quiet-periods for the ADC.
2.  **SRAM Memory Preservation:** Every literal string printed to the Serial Monitor **MUST** be encapsulated in the `F()` macro (e.g., `Serial.println(F("Hello"))`) to prevent SRAM memory exhaustion.
3.  **Non-Blocking Diagnostics:** All telemetry and diagnostic logs in the Serial Monitor must be wrapped in `if (Serial)` check, so they execute instantly and bypass printing overhead entirely when no USB Serial Monitor is connected.
4.  **Hardware-Driven Calibration:** Battery calibration multipliers (`RX_CALIBRATION`, `TX_CALIBRATION`) should be adjusted using the live suggestions generated automatically in the Serial Monitor telemetry block.
5.  **Relay Failsafe Enforcement:** The welding output relays (`PIN_REL1`, `PIN_REL2`) and the PWM signal must be immediately de-energized/locked to `0` when a signal loss (LoRa timeout) or Standby state is detected.
6.  **Embedded C++ Standard:** Always write `.ino` firmware using clean, lightweight **Embedded C++** patterns. Avoid dynamic memory allocation (no `new`/`delete` or `std::vector`), prevent runtime heap fragmentation, prefer static or stack-allocated memory, and implement object-oriented abstractions or standard patterns only when they do not introduce memory overhead or compromise firmware execution speed.

