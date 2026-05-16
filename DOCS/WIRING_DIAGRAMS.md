# WIRING DIAGRAMS: WIRELESS WELDER PEDAL v1.0 (Integrated LoRa32u4)

> **Technical Data Source:** `HARDWARE_LIST_CLASSIFIED.csv` + `transmitter.ino` / `receiver.ino`

---

> [!CAUTION]
> **CONNECT ANTENNA BEFORE POWERING ON:** Never power or turn on the LoRa32u4 board without the antenna connected. Operating the system without a load (antenna) can permanently damage the SX1278 radio chip due to power reflection.

> [!WARNING]
> **BATTERY POLARITY REVERSED:** The stock 103048 LiPo batteries usually come with the JST connector wires inverted relative to this specific board. You **MUST** swap the pins on the battery connector before plugging it in. On the LoRa32u4 controller, the **positive (+)** pin is on the **LEFT**, and the **negative (-)** pin is on the **RIGHT** (when looking at the connector from the edge of the board). Failure to do this will cause a reverse-polarity short!

---

## 1. TRANSMITTER (Pedal)

The transmitter integrates the MCU (ATmega32u4 @ 8MHz) and the LoRa SX1278 radio on a single PCB. The **MS-105 limit switch (SPDT)** activates the system: when pressed, it turns on the VL53L4CD laser sensor via XSHUT and enables the transmission of full telemetry to the receiver. The 103048 LiPo (3.7V) powers everything from the board's integrated JST connector.

### Wiring Diagram - Transmitter v7.0
![Transmitter Wiring Diagram](file:///Users/ARICF/Documents/PROYECTOS/WELDER%20PEDAL/WEB/wireless-pedal/ai-knowledge-core/docs/hardware/transmitter_wiring.png)

### Visual Component Reference
![Component Reference - LoRa32u4](file:///Users/ARICF/Documents/PROYECTOS/WELDER%20PEDAL/WEB/wireless-pedal/ai-knowledge-core/docs/hardware/component_lora32u4.png)
![Component Reference - Peripherals](file:///Users/ARICF/Documents/PROYECTOS/WELDER%20PEDAL/WEB/wireless-pedal/ai-knowledge-core/docs/hardware/component_peripherals.png)

### Connection Table - Transmitter

| Component | LoRa32u4 Pin | Wire Color | Function |
| :--- | :---: | :---: | :--- |
| **VL53L4CD - VIN** | 3V3 | 🔴 Red | 3.3V Power (Logic Voltage 2.6V-3.5V) |
| **VL53L4CD - GND** | GND | ⚫ Black | Ground |
| **VL53L4CD - SCL** | Pin 3 (SCL) | 🔵 Blue | I2C Clock |
| **VL53L4CD - SDA** | Pin 2 (SDA) | 🟡 Yellow | I2C Data |
| **MS-105 - COM** | Pin 11 | 🟢 Green | Trigger/Safety (INPUT_PULLUP, Active LOW) |
| **MS-105 - NO** | GND | ⚫ Black | Closes on press -> LOW on Pin 11 |
| **LIPO-103048-1500 (+)** | JST BAT (LEFT) | 🔴 Thick Red | 3.7V Power (Check Polarity!) |
| **LIPO-103048-1500 (-)** | JST BAT (RIGHT)| ⚫ Thick Black | Ground |
| **Battery Monitor** | A9 (analog) | - | Internal voltage divider |
| **Power Button - Switch** | EN / GND | ⚫ Black | M10 Button (Latching) - Connect to EN / GND |
| **Power Button - LED (+)**| Pin 12 | 🔴 Red | Software controlled Status LED (Solid/Blink/Coma) |
| **Power Button - LED (-)**| GND | ⚫ Black | Ground |

### Component Specifications (Transmitter)

| Component | Model / MPN | Key Specifications |
| :--- | :--- | :--- |
| **MCU + LoRa** | DIYmall LoRa32u4 RA-02 | ATmega32u4 @ 8MHz · SX1278 433MHz · up to 1KM · 3.3V logic |
| **ToF Sensor** | VL53L4CDV0DH/1 | ToF I2C · 1-1300mm · ±3% · Logic Voltage 2.6V-3.5V |
| **Limit Switch** | MS-105 (SPDT) | 3A 250VAC / 5A 125VAC · 1NO+1NC · 100,000 cycles |
| **Battery** | LiPo 103048-1500mAh | 3.7V nom. (4.2V full) · 10x30x48mm · rechargeable |
| **Return Springs** | 304 Stainless V-type | ø1.0mm · 6 turns · 60° · mechanical pedal reset |

> [!IMPORTANT]
> **The XSHUT Conflict:** The orange wire (XSHUT / LPN) that previously went to Pin 6 has been **removed from the design**. Connecting it caused a resistance conflict with the LoRa32u4 board that choked the I2C bus. By leaving it disconnected, the sensor turns on automatically via its internal pull-up resistor.
> **Trigger (Pin 11):** The trigger has been moved from Pin 5 to Pin 11 to avoid internal conflicts with the LoRa radio (DIO1).

> [!CAUTION]
> **PIN 5 ADVISORY:** Pin 5 of the LoRa32u4 is internally connected to the DIO1 pin of the LoRa module. DO NOT use it as a general input/output to avoid communication failures.

---

## 2. RECEIVER (Welding Machine)

The receiver picks up the LoRa packet from the pedal and converts the distance into a PWM -> analog (0-10V) signal to control the welder's current. It uses two LR7843 MOSFET modules (opto-isolated, 30A): one as a **firing relay** for the torch and another to **isolate the DAC module's GND**, ensuring a true 0.00V when the pedal is at rest.

### Wiring Diagram - Receiver (GND Isolation)
![Receiver Wiring](file:///Users/ARICF/Documents/PROYECTOS/WELDER%20PEDAL/WEB/wireless-pedal/ai-knowledge-core/docs/hardware/receiver_wiring.png)

### Connection Table - Receiver

| Component | LoRa32u4 Pin | Wire Color | Function |
| :--- | :---: | :---: | :--- |
| **SSD1306 OLED - SDA** | Pin 2 (SDA) | 🟡 Yellow | I2C Telemetry (distance, battery, status) |
| **SSD1306 OLED - SCK** | Pin 3 (SCL) | 🔵 Blue | I2C Clock |
| **SSD1306 OLED - VCC** | 3V3 | 🔴 Red | 3.3V Power |
| **SSD1306 OLED - GND** | GND | ⚫ Black | Ground |
| **MOSFET #1 - PWM (+)** | Pin 12 | 🟣 Purple | Relay 1 - Fires the welding torch |
| **MOSFET #1 - GND (-)** | GND | ⚫ Black | Control Ground (MCU) |
| **MOSFET #2 - PWM (+)** | Pin 11 | 🟠 Orange | Relay 2 - Connects/disconnects DAC GND |
| **MOSFET #2 - GND (-)** | GND | ⚫ Black | Control Ground (MCU) |
| **PWM-to-DAC - (PWM)** | Pin 10 | 🟡 Yellow | PWM Signal Input (from MCU) |
| **PWM-to-DAC - (GND L)** | GND | ⚫ Black | Logic Ground (MCU) |
| **PWM-to-DAC - (VIN)** | GX12 Pin 5 | 🟡 Yellow | Power Supply (+) from Welder (10-12V) |
| **PWM-to-DAC - (GND In)** | MOSFET #2 (L) | ⚫ Black | Switched Ground (Galvanic Isolation) |
| **PWM-to-DAC - (Vo)** | GX12 Pin 7 | 🟢 Green | 0-10V Analog Output to welder |
| **LIPO-103048-1500 (+)** | JST BAT (LEFT) | 🔴 Thick Red | 3.7V Power (Check Polarity!) |
| **LIPO-103048-1500 (-)** | JST BAT (RIGHT)| ⚫ Thick Black | Ground |
| **GX12-5P Connector** | Male Plug | - | [See Pinout Table below] |
| **Power Button - Switch** | EN / GND | ⚫ Black | M10 Button (Latching) - Connect to EN / GND |
| **Power Button - LED (+)**| Pin A9 | 🔴 Red | Software controlled Status LED (Solid/Off) |
| **Power Button - LED (-)**| GND | ⚫ Black | Ground |

### Aviation Connector Pinout (GX12-5P Male)

This connector links the Receiver unit to the welding machine's remote/pedal port. Note that while it uses a 5-pin physical layout, the pins are numbered 2, 3, 5, 6, and 7 based on the industrial standard for this specific machine.

| Pin (GX12) | Wire Color | Function | Receiver Connection |
| :---: | :---: | :--- | :--- |
| **2** | 🔴 Red | Trigger Switch (+) | **MOSFET #1 - (+) / LOAD** (Bridged) |
| **3** | ⚫ Black | Trigger Switch (-) | **MOSFET #1 - (-)** |
| **5** | 🟡 Yellow | Remote Ref. (10-12V) | **PWM-to-DAC - (VIN)** (⚪ White in Thermal Ark) |
| **6** | 🟤 Brown | Remote GND (Min) | **MOSFET #2 - (-)** |
| **7** | 🟢 Green | Remote Wiper (Sig) | **PWM-to-DAC - (Vo) (0-10V)** |

> [!IMPORTANT]
> **CRITICAL JUMPER FOR TORCH TRIGGER:** To use MOSFET #1 as an isolated "dry contact" for the welding machine trigger, you **MUST** bridge (solder a jumper wire) between the **`+`** and **`LOAD`** pins on the MOSFET's output side. The welder's positive wire then connects to this bridged point, and the negative wire to the **`-`** pin.

### Component Specifications (Receiver)

| Component | Model / MPN | Key Specifications |
| :--- | :--- | :--- |
| **MCU + LoRa** | DIYmall LoRa32u4 RA-02 | ATmega32u4 · SX1278 433MHz · 3.3V logic · integrated LiPo charging |
| **OLED Display** | SSD1306 0.96" | 128x64px · I2C · 4 pins (VCC, GND, SCL, SDA) · 3.3V-5V |
| **MOSFET Driver** | LR7843 (x2) | LR7843 Chip · 30A max · Opto-isolated · 3V-20V PWM control |
| **DAC Converter** | PWM-to-DAC 0-10V | PWM Input · 0-5V or 0-10V Output · 12V-30V Power · Multi-turn Pot |
| **Battery** | LiPo 103048-1500mAh | 3.7V nom. · 10x30x48mm · integrated PCM |
| **Ext. Connector** | GX12-5P Male | Industrial aviation · 12mm · 5 contacts · Zinc/Nickel |

### MOSFET Wiring Detail (LR7843)

Each MOSFET module has two distinct sides. Based on your board's silk-screen:

**1. Input Side (Digital Interface / MCU):**
*   **PWM (+):** Connect to the LoRa32u4 signal pin (**Pin 12** for Torch or **Pin 11** for DAC).
*   **GND (-):** Connect to the common LoRa32u4 GND.

**2. Output Side (Power / LOAD):**
The LR7843 MOSFET performs **Low-Side Switching**, meaning it opens or closes the path to the negative (`-`).

#### MOSFET #1: Torch Trigger (Relay Trigger - ISOLATED)
*   **Pin `+` (Output):** Connect to the welder's positive trigger **GX12 Pin 2 (🔴 Red)**, AND bridge it to the **`LOAD`** pin (connect both terminals).
*   **Pin `LOAD` (Output):** Physically bridged to the `+` pin via a solder jumper.
*   **Pin `-` (Output):** Connect to the welder's trigger return wire **GX12 Pin 3 (⚫ Black)**.
*   *Note: This creates a "dry contact" relay effect that keeps the welder's high voltage completely isolated from the Arduino electronics.*

#### MOSFET #2: DAC Ground Isolation (FULLY ISOLATED)

In this configuration, the DAC is powered and grounded **only** by the welder's reference pins, ensuring zero electrical noise return to the Arduino.

*   **Pin `+` (Output):** Connect to **PWM-to-DAC - (VIN)** 🟡 Yellow wire.
*   **Pin `LOAD` (Output):** Connect to **PWM-to-DAC - (GND In)** ⚫ Black wire.
*   **Pin `-` (Output):** Connect to the welder's remote ground **GX12 Pin 6 (🟤 Brown)**.
*   *Note: When Pin 11 is LOW, the MOSFET opens the circuit and physically removes the path between the DAC and the Welder's ground. The DAC powers up using the 10-12V from GX12 Pin 5.*

---

### Ground Isolation Details (GND Isolation)

> [!IMPORTANT]
> The **MOSFET LR7843 #2 (Pin 11)** does not cut the PWM signal; instead, it **physically connects/disconnects the PWM-to-DAC module's GND pin**. It has been moved to Pin 11 for wiring convenience. This ensures a **true 0.00V** at the analog output to the welder when the pedal is at rest.
