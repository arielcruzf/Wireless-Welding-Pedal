# TESTING AND TROUBLESHOOTING LOG

This document chronologically records the problems encountered, hypotheses proposed, and solutions applied during the development of the **Wireless Welder Pedal** firmware.

---

## PHASE 1: Screen Stabilization and Basic Trigger (v7.0 - v7.2)
*   **Initial State:** The trigger worked (sent 0% - 100% PWM), but the OLED screen rendering using heavy bitmaps (`epd_bitmap`) caused memory crashes and graphical overlaps.
*   **Problem:** When the transmitter was disconnected, the screen showed the "X" over the previous icons.
*   **Solution:** The **Receiver (`receiver.ino`)** interface was rewritten to use pure geometric rendering (`drawRect`, `fillRect`). Mathematical parameters were standardized, achieving a perfect, fluid layout without memory loss.

---

## PHASE 2: Communication Crashes and I2C Loop (v7.6 - v8.7)
*   **Problem:** When connecting the VL53L4CD ToF sensor and pressing the trigger, the system disconnected completely (an "X" appeared on the welder's screen).
*   **Hypothesis 1 (Electrical Noise):** It was thought that physically turning on the laser (`XSHUT`) generated noise. Delays and an "I2C Shield" (I2C Ping) were implemented to ignore the sensor if it did not respond, preventing the program from hanging.
*   **Result:** The system stopped disconnecting when the trigger was pressed, but the laser still read "0%".
*   **Hypothesis 2 (I2C Buffer Limit):** The ATmega32u4 architecture was investigated. It was discovered that the official `STM32duino` library attempted to send the laser firmware in blocks larger than the 32-bytes allowed by the AVR `Wire` library, causing a silent infinite hang.
*   **Solution:** The code was migrated to the **"VL53L4CD by Pololu"** library, which is designed with automatic chunking and software timeouts, ensuring the chip never freezes.

---

## PHASE 3: Wireless Diagnostics and Voltage Drop (Brownout) (v9.0 - v9.2)
*   **Post-Pololu Problem:** Despite the library change, the sensor still reported 0%, and the transmitter board's USB port would randomly disconnect from the Arduino IDE when running the code.
*   **Diagnostic Hypothesis:** Since the USB disconnected and the Serial Monitor could not be used, a **"Wireless I2C Scanner" (v9.1)** was created. This firmware mapped I2C states to specific percentages sent via LoRa to the screen (0% = Dead, 51% = OK 0x29). The result was "0%", indicating the sensor was electrically dead at startup.
*   **Root Cause (Radio Brownout):** The power topology was analyzed. When powering the board **only via USB**, the internal 3.3V regulator suffered a massive voltage drop (Brownout) when the LoRa radio module attempted to transmit at maximum power (17dBm / ~120mA). 
    *   This voltage drop turned off the VL53L4CD sensor (which requires stable 3.3V) and reset the ATmega32u4 (disconnecting the USB).
*   **Applied Solution (v9.2):** 
    1. The LoRa radio was forced to operate at **Low Power (`LoRa.setTxPower(2)`)**.
    3. **Hardware Rule:** It was established that for continuous transmission testing or final use, the LiPo battery must be connected to absorb the current spikes that the USB cannot manage.

---

## PHASE 4: The XSHUT Pin Conflict (Final Awakening)
*   **Problem:** Despite stable voltage and the anti-freeze Pololu library, the I2C scanner still failed to detect the sensor (reading 0%).
*   **Hardware Hypothesis:** The orange wire (`XSHUT` / LPN) was connected to Pin 6 of the Arduino. If the Arduino could not pull that pin to a clean 3.3V (due to an internal conflict with the LoRa32u4 board, a shared pull-down resistor, or a pin current limit), the sensor remained eternally off in "Shutdown" mode.
*   **Definitive Test:** It was ordered to physically disconnect the orange wire (Pin 6) and leave the sensor's `XSHUT` pin floating.
*   **Result:** Immediate Success! The I2C scanner instantly detected the sensor at address `0x29` (showing 51% on the screen).
*   **Architectural Conclusion:** The AliExpress VL53L4CD modules have an internal pull-up resistor on the `XSH` pin. By leaving it disconnected, the sensor auto-ignites safely. Pin 6 of the LoRa32u4 board was choking this signal.
*   **Final Action:** All `XSHUT` pin control code was removed from the transmitter. The sensor now operates freely, commanded only by I2C read requests.

---

## PHASE 5: Final Calibration and Advanced Filtering (v10.3)
*   **Range Adjustment:** The working range was standardized to **15mm (100%) to 50mm (0%)**.
*   **Dual Filter (Median + EMA):**
    *   **Median Filter (5 samples):** Implemented to eliminate ±3mm reading noise (jitter). This filter ignores spikes and maintains the central value.
    *   **EMA Filter (0.4):** Applied over the median value to ensure smooth transitions.
*   **Standby Mode:** Automatic LoRa radio disconnection was added after 5 minutes of inactivity, with a **flashing Hourglass icon (100ms)** on the receiver to indicate this state.
*   **Latency Optimization:** The sensor timing budget was adjusted to 50ms and the transmission loop to 30ms.

---

## PHASE 6: OLED Interface Refinement and RSSI Calibration
*   **Geometric Interface (OLED):** The Receiver's screen design was updated using `drawRoundRect` (rounded corners) for the 4 information blocks (PWM, Antenna, Bat TX, Bat RX). Dynamic mathematical centering for the PWM percentage text was implemented, and the disconnection 'X' icon size was reduced.
*   **LoRa Antenna Calibration:** The RSSI signal mapping was adjusted. Since LoRa technology operates efficiently with very weak signals, a range of `-110dBm` (0 bars) to `-60dBm` (5 full bars) was established, achieving a realistic visual representation of link strength at normal working distances (5-10 meters).
