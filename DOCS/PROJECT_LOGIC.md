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

## 7. Firmware Logical Architecture (Release V1.1 Standard)
Wireless communication between both LoRa32u4 boards operates under an ultra-optimized master-slave (unidirectional) system, where the **Transmitter** (Pedal) dictates physical status and the **Receiver** (Machine) executes power and displays visual status on the OLED screen.

### 7.1. Transmitter & Receiver File Directory
*   **Transmitter Firmware:** Located in [transmitter_v1.1.ino](file:///Users/ARICF/Documents/PROYECTOS/WELDER%20PEDAL/FIRMWARE/transmitter_v1.1/transmitter_v1.1.ino).
*   **Receiver Firmware:** Located in [receiver_v1.1.ino](file:///Users/ARICF/Documents/PROYECTOS/WELDER%20PEDAL/FIRMWARE/receiver_v1.1/receiver_v1.1.ino).

### 7.2. Trigger Flow and Action ("Dumb Node, Smart Controller" Architecture)
*   **Transmitter (`transmitter_v1.1.ino`):** Pin 11 acts as the master limit switch (using Pin 10 as virtual GND). When closed (pedal pressed), it boots the VL53L4CD laser sensor. The transmitter applies raw Median & EMA filtering and compresses the raw distance (10-150mm) into a single byte (`0..254`). If standby is triggered, it radiates a farewell byte `255`. If the laser fails, it falls back to 150mm (mapped). Loop latency is under 10ms.
*   **Receiver (`receiver_v1.1.ino`):** Constantly listens on 433MHz. When it receives a packet, it decodes `laserM` (if `255` -> Standby, else reconstructs millimeters using `map(laserM, 0, 254, 10, 150)`), unpacks flags for the physical switch, and scales battery raw readings. All welding calibration boundaries (`PEDAL_UP_MM` and `PEDAL_DOWN_MM`) are handled here, mapping the reconstructed distance to active PWM output (0-244).

### 7.3. The 3-Byte Compressed Payload
To minimize RF Time-on-Air (ToA) and reduce power consumption, Release V1.1 implements a highly compressed 3-byte payload structure:
```cpp
struct __attribute__((packed)) PedalDataPayload {
  uint8_t laserM;  // 0..254 = mapped distance (10..150mm), 255 = Standby (999mm)
  uint8_t flags;   // Bit 0: switchClosed (trigger), Bits 1..7: unused
  uint8_t batRaw;  // Raw ADC battery reading divided by 2
};
```
*   **Battery Scaling:** By dividing the 10-bit raw ADC reading (`250..400`) by 2 on the transmitter and multiplying by 2 on the receiver, we save 1 byte. This preserves 100% of the receiver's EMA smoothing (`sT`) and alarm triggers with a resolution loss of only ~15mV.

### 7.4. Advanced Filtering Chain (Anti-Jitter)
To ensure the welding arc is completely smooth, the transmitter applies two sequential mathematical filters before encoding the payload:
1.  **Median Filter (3 Samples):** Discards extreme transient spikes, keeping the central value to eliminate high-frequency noise.
2.  **EMA Filter (Exponential Moving Average - Factor 0.15 / 0.85):** Averages the median result with recent history to deliver smooth transitions during active foot travel.

### 7.5. The 4 Ecosystem States (Failsafe Logic and OLED)
The **Receiver** continuously evaluates data flow:
1.  **CONNECTED (Steady Signal Bars):** Signal is stable. Packets arrive smoothly (<200ms apart).
2.  **HOLDING (Flashing Signal Bars):** Packets are delayed (>200ms) but within safety limit. Holds PWM active to avoid killing the arc during minor interference.
3.  **DISCONNECTED ('X' Icon):** Safety timeout passed (Failsafe = 1000ms) without receiving data. Instantly cuts relays and PWM to 0%.
4.  **STANDBY (Flashing Hourglass):** After 3 minutes of inactive foot travel, the transmitter turns off its radio to save battery. It sends a Farewell Burst with `laserM = 255`. Upon receiving, the receiver enters safe sleep mode (PWM = 0) and dims the OLED brightness to minimum.
    *   **Heartbeat Protocol:** The sleeping transmitter wakes up every 2 seconds via WDT, transmits a `laserM = 255` heartbeat packet, and sleeps again. If the receiver spends more than 10 seconds without a packet, it drops to the DISCONNECTED state.

### 7.6. Power Saving, USB Detach & Hardware Waking
*   **Deep Sleep:** In deep sleep (>10 minutes inactive), the transmitter turns off LoRa, cuts laser power (XSHUT = LOW), disables the virtual USB transceiver (`USBCON = 0`), and powers down the MCU. It wakes exclusively via hardware pin change interrupts (**PCINT7**) on the pedal switch or mode button.
*   **VBUS Hot-Plug re-enumeration:** Both firmware modules monitor VBUS (`USBSTA & (1 << VBUS)`). When USB is connected, they call `USBDevice.detach()`, wait 500ms, and call `USBDevice.attach()` to guarantee clean COM port re-enumeration for IDE flashing without locking.

---
**Project Status:** ✅ Full Premium Interface | ✅ Global RAG System integrated | ✅ Automated CSV Ingestion | ✅ Stabilized and Optimized Firmware Release V1.1.
> **New Golden Rule:** The AI only learns what the developer validates via the `update ia` command, avoiding noise from unconfirmed interactions.
