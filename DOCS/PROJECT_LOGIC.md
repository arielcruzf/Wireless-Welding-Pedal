# TECHNICAL LOG: WIRELESS WELDER PEDAL WEB PROJECT

This document summarizes the logical architecture and workflow implemented for the premium "One-Page" platform and its **RAG (Retrieval-Augmented Generation)** intelligence ecosystem.

---

## 1. Infrastructure and Environment (The Foundations)
Optimized configuration for agile and secure development between local environments and cloud deployment.
*   **Next.js 15 (App Router):** SPA engine, optimized for SEO and media loading (4K videos/heavy assets).
*   **Security:** Management of `GEMINI_API_KEY` and Upstash credentials via `.env.local` (local) and environment variables (Vercel).
*   **Tooling:** Connection to **Stitch MCP** servers for mechanical UI generation and **tsx** for data pipeline execution.

## 2. AI Brain: ai-knowledge-core (Global RAG)
The knowledge core is not static; it dynamically feeds off the entire project repository.

*   **Power Sources:**
    *   **Master CSV (`../HARDWARE/`):** The classified `.csv` file is the sole source of truth for components and specs.
    *   **Firmware (`../CODE/`):** `.ino` source code of the controllers for technical reasoning.
    *   **Mechanics (`../3D/`):** Discovery of STL/STEP models for assembly.
    *   **Multimedia (`../MEDIA/`):** Visual references of the manufacturing process.
    *   **Living Context:** Instructions from the **Prompt Box** and direct feedback from the **Frontend Chat**.

*   **Intelligence Infrastructure (Upstash Cloud):**
    *   **Vector Indexing:** Use of `gemini-embedding-001` (1536 dims) in Upstash Vector for semantic search.
    *   **Session Memory:** History persistence in Upstash Redis to maintain the thread of complex diagnostics.

## 3. Work Methodology: Intentional Ingestion
We have moved from passive learning to controlled and **Intentional Ingestion** to guarantee knowledge quality.

*   **"update ia" Command:** This is the central trigger for learning. When executed (via `ingest-all.ts`):
    1.  Reads the master CSV and synchronizes the manual [HARDWARE_LIST.md](file:///Users/ARICF/Documents/PROYECTOS/WELDER%20PEDAL/WEB/ai-knowledge-core/docs/hardware/HARDWARE_LIST.md).
    2.  Recursively scans the Hardware, Code, 3D, and Media folders.
    3.  Updates vectors in the cloud, reporting every new file detected.
*   **Sync Control:**
    *   `npx tsx ai-knowledge-core/ingest-all.ts --watch`: Listens for real-time changes.
    *   **Postbuild:** Guaranteed automatic synchronization on every production deployment.

## 4. Verification and Quality (Tooling)
Diagnostic tools to ensure the AI's "truth" matches physical reality.
*   **Test-Suite (`test-gemini.js`):** Validates connection health, API quotas, and embedding accuracy.
*   **Docs Sync:** Automatic verification that wiring diagrams and component tables match the CSV inventory.

## 5. Deployment Pipeline (CI/CD)
*   **Versioning:** GitHub acts as the central hub for code, RAG logic, and 3D models.
*   **Passwordless Git Authentication (macOS Keychain & PAT):** The local repository is configured with a permanent GitHub Personal Access Token (PAT Classic - No Expiration) embedded in the remote URL. This allows the local Mac to authenticate, push commits, and update release tags (`V1.0`, `V0.9`) seamlessly without ever prompting the developer for a username or password.
*   **Edge Hosting (Vercel):** Continuous deployment with high-availability infrastructure and serverless function execution for the chat.

## 6. Diagramming & Presentation Skills (LLM Output)
The assistant has been equipped with advanced visual capabilities to facilitate technical understanding within the chat.
*   **UI Capabilities (Client Side):**
    *   **Markdown Pro:** `react-markdown` integration with GFM table support.
    *   **Mermaid.js:** Dynamic rendering of flowcharts and connection diagrams directly from AI-generated code blocks.
    *   **KaTeX:** Support for mathematical and physical formulas (LaTeX notation) for power and electronics calculations.
    *   **C++ Coding Skill:** Ability to generate and correct firmware for LoRa32u4 boards, integrating sensor logic and SX1278 radio protocols.
*   **Aesthetic Sync:** "Neutral/white" theme configuration for diagrams and tables to maintain the premium and clean aesthetic of the original website.
*   **Prompt Training:** Injection of specific rules into the `SYSTEM_PROMPT` to prioritize the use of these visual tools over plain text.

---

## 7. Firmware Logical Architecture (Transmitter and Receiver)
Wireless communication between both LoRa32u4 boards operates under an ultra-optimized master-slave (unidirectional) system, where the **Transmitter** (Pedal) dictates physical status and the **Receiver** (Machine) executes power and displays visual status on the OLED screen.

### 7.1. Trigger Flow and Action ("Dumb Node, Smart Controller" Architecture)
*   **Transmitter (`transmitter.ino`):** Operates as a "Dumb Node". Pin 11 acts as the master switch. When closed (pedal pressed), it wakes up the VL53L4CD laser sensor. The transmitter applies internal mathematical filters to the raw laser reading and radiates this *unconstrained raw distance* (along with switch status) in a LoRa packet at maximum power (12dBm). By removing calibration limits from the pedal, the firmware becomes universal and safer. If the laser fails while pressing the pedal, it sends a safe fallback distance of 150mm (0% Amps). Total loop latency is under 10ms.
*   **Receiver (`receiver.ino`):** Operates as the "Smart Controller" and constantly listens on the 433MHz frequency. Upon receiving a valid packet, if the "switch" is closed, it activates the MOSFETs (Relays). **Crucially, all pedal calibration (`DIST_REPOSO_MM` and `DIST_FONDO_MM`) is exclusively managed here.** It maps the received raw millimeters to a precise PWM output (0-244) to control the welder's DAC. This makes adjusting pedal sensitivity infinitely easier, as you only need to reprogram the receiver unit on the desk.

### 7.2. Advanced Filtering Chain (Anti-Jitter)
To ensure the welding arc is smooth and eliminate the inherent "tremor" of ToF sensors at close range, the transmitter applies three sequential mathematical filters before emitting the radio signal:
1.  **Median Filter (3 Samples):** Collects the last 3 laser measurements and discards extreme peaks, keeping the central value. This clears visual "noise" almost instantaneously.
2.  **EMA Filter (Exponential Moving Average - Factor 0.6):** Takes the median result and averages it mathematically with recent history. The 0.6 factor provides an "electric" and agile reaction to the foot, but with a stepped and smooth transition.
3.  **Hysteresis Filter (Dead Zone):** If the resulting distance variation is 1mm or less, the code ignores the change. This locks the percentage number on the screen (and thus the PWM delivery) when the welder keeps their foot completely static.

### 7.3. The 4 Ecosystem States (Failsafe Logic and OLED)
The **Receiver** continuously evaluates data flow to ensure operator safety (Intelligent Failsafe) and represents it visually as follows:

1.  **CONNECTED (Steady Signal Bars):** The pedal is in use and the signal is stable. LoRa packets arrive smoothly (<200ms apart). The screen draws the actual signal strength from the antennas (with calibration adjusted to -115dBm to represent LoRa's high sensitivity).
2.  **HOLDING (Flashing Signal Bars):** More than 200ms have passed since the last packet, but less than the safety limit. The system senses a micro-interference. It keeps the PWM power active temporarily to avoid shutting down the welding arc and warns visually by flashing the antenna bars.
3.  **DISCONNECTED ('X' Icon):** The strict safety limit has passed (Failsafe = 1000ms) without receiving data from the pedal. The receiver instantly shuts off the Relays and cuts the PWM to 0%. The screen shows a large 'X' indicating the link has been lost or the pedal has been abruptly turned off.
4.  **STANDBY (Flashing Hourglass):** If the pedal is not pressed for 5 minutes, the transmitter turns off its radio to save the LiPo battery. Before doing so, it sends a "Farewell Burst" (10 consecutive packets with the secret code `laserDist = 999`). 
    *   Upon receiving `999`, the receiver enters safe sleep mode (PWM = 0) and replaces the antenna with an hourglass.
    *   **Heartbeat Protocol:** To avoid "false standbys," the sleeping pedal wakes up every 60 seconds just to send 3 warning packets ("I'm still here") and goes back to sleep. If the machine spends more than 70 seconds without hearing this heartbeat, it assumes the pedal has run out of battery and exits Standby to show the 'X' for Disconnected.

### 7.4. Hardware Programming Limitation (Important)
*   **Physical Switch Constraint:** If the physical power switch of either the **Transmitter** or the **Receiver** is in the **OFF** position, it is **impossible to upload code** via the Arduino IDE. The switch must be turned **ON** for the computer to recognize the USB COM port and allow the bootloader to flash new firmware.

### 7.5. Arduino IDE Board Selection (Timing Accuracy)
*   **F_CPU Mismatch:** The physical LoRa32u4 boards run at **8MHz (3.3V)**. If a 16MHz board (like "Arduino Leonardo") is selected in the Arduino IDE during firmware upload, all time-dependent functions (`millis()`, `delay()`) will execute at exactly half speed (e.g., a 5-minute standby will take 10 minutes).
*   **Correct Board:** To ensure accurate timing and I2C speeds, always install the "Adafruit AVR Boards" package and select **"Adafruit Feather 32u4"** (which defaults to 8MHz) before compiling and uploading the code.

### 7.6. Battery Alarm System (Buzzer)
*   **Non-Blocking Audio:** The Receiver uses an asynchronous state machine (`BuzzerAlarm` class) on Pin 6 to emit audible warnings without interrupting the main processing loop or using blocking `delay()` functions.
*   **Trigger Logic:** The system continuously monitors the battery percentages of BOTH the Transmitter and Receiver.
    *   **10% Warning:** If either battery drops to $\le 10\%$, it emits 3 long buzzes (1s ON, 1s OFF). This alarm is triggered only once and won't re-trigger unless the battery climbs above 12% (hysteresis) and drops again.
    *   **5% Critical Warning:** If either battery drops to $\le 5\%$, it emits 5 long buzzes (1s ON, 1s OFF). The hysteresis threshold to reset this alarm is 7%.

---
**Project Status:** ✅ Full Premium Interface | ✅ Global RAG System integrated | ✅ Automated CSV Ingestion | ✅ Stabilized and Optimized Firmware v10.4.
> **New Golden Rule:** The AI only learns what the developer validates via the `update ia` command, avoiding noise from unconfirmed interactions.
