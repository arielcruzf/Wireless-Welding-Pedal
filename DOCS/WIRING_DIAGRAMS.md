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
| **VL53L4CD - LPN/XSHUT**| Pin 6 | 🟣 Purple | Laser Sleep/Wake Control (Open-Drain) |
| **MS-105 - COM** | Pin 11 | 🟢 Green | Trigger/Safety (INPUT_PULLUP, Active LOW) |
| **MS-105 - NO** | Pin 10 | ⚫ Black | Virtual GND (Set to LOW by MCU) |
| **LIPO-103048-1500 (+)** | JST BAT (LEFT) | 🔴 Thick Red | 3.7V Power (Check Polarity!) |
| **LIPO-103048-1500 (-)** | JST BAT (RIGHT)| ⚫ Thick Black | Ground |
| **Battery Monitor** | A0 (analog) | - | Internal voltage divider |
| **Power Button - Switch (+)**| Pin A1 | 🔴 Red | Momentary Power Button (INPUT_PULLUP, Active LOW) [Terminal A4 on Button] |
| **Power Button - Switch (-)**| Pin A5 | ⚫ Black | Virtual Ground for Switch (MCU set to LOW) [Terminal A3 on Button] |
| **Power Button - LED (+)**| Pin A3 | 🔴 Red | Software controlled Status LED (Blinks in Standby) [Terminal A1 on Button] |
| **Power Button - LED (-)**| Pin A4 | ⚫ Black | Virtual Ground for LED (MCU set to LOW) [Terminal A2 on Button] |

### Component Specifications (Transmitter)

| Component | Model / MPN | Key Specifications |
| :--- | :--- | :--- |
| **MCU + LoRa** | DIYmall LoRa32u4 RA-02 | ATmega32u4 @ 8MHz · SX1278 433MHz · up to 1KM · 3.3V logic |
| **ToF Sensor** | VL53L4CDV0DH/1 | ToF I2C · 1-1300mm · ±3% · Logic Voltage 2.6V-3.5V |
| **Limit Switch** | MS-105 (SPDT) | 3A 250VAC / 5A 125VAC · 1NO+1NC · 100,000 cycles |
| **Battery** | LiPo 103048-1500mAh | 3.7V nom. (4.2V full) · 10x30x48mm · rechargeable |
| **Return Springs** | 304 Stainless V-type | ø1.0mm · 6 turns · 60° · mechanical pedal reset |

> [!TIP]
> **XSHUT/LPN Resolution (v1.0 POWER-MASTER):** The LPN pin of the sensor is now connected to **Pin 6** using a purple wire. Previous I2C conflicts have been resolved in firmware v1.0 using an Open-Drain software configuration (input for ON, output LOW for OFF), allowing maximum power savings during deep sleep.
> **Trigger (Pin 11):** The trigger has been moved from Pin 5 to Pin 11 to avoid internal conflicts with the LoRa radio (DIO1), utilizing Pin 10 as a software-controlled virtual GND.

> [!CAUTION]
> **PIN 5 ADVISORY:** Pin 5 of the LoRa32u4 is internally connected to the DIO1 pin of the LoRa module. DO NOT use it as a general input/output to avoid communication failures.

---

## 2. RECEIVER (Welding Machine)

The receiver picks up the LoRa packet from the pedal and converts the distance into a PWM -> analog (0-10V) signal to control the welder's current. It uses two 4-pin **Photocoupler TLP222A (photorelays)**: one as a **firing relay** for the torch and another to **isolate the DAC module's GND**, ensuring a true 0.00V when the pedal is at rest.

### Wiring Diagram - Receiver (GND Isolation)
![Receiver Wiring](file:///Users/ARICF/Documents/PROYECTOS/WELDER%20PEDAL/WEB/wireless-pedal/ai-knowledge-core/docs/hardware/receiver_wiring.png)

### Connection Table - Receiver

| Component | LoRa32u4 Pin | Wire Color | Function |
| :--- | :---: | :---: | :--- |
| **SSD1306 OLED - SDA** | Pin 2 (SDA) | 🟡 Yellow | I2C Telemetry (distance, battery, status) |
| **SSD1306 OLED - SCK** | Pin 3 (SCL) | 🔵 Blue | I2C Clock |
| **SSD1306 OLED - VCC** | 3V3 | 🔴 Red | 3.3V Power (from board 3.3V regulator) |
| **SSD1306 OLED - GND** | GND | ⚫ Black | Ground |
| **MOSFET #1 - PWM (+)** | Pin A5 | 🟣 Purple | Relay 1 - Fires the welding torch |
| **MOSFET #1 - GND (-)** | Pin 10 | ⚫ Black | Virtual Control Ground (MCU set to LOW) |
| **MOSFET #2 - PWM (+)** | Pin A3 | 🟠 Orange | Relay 2 - Connects/disconnects DAC GND |
| **MOSFET #2 - GND (-)** | Pin 12 | ⚫ Black | Virtual Control Ground (MCU set to LOW) |
| **PWM-to-DAC - (PWM)** | Pin 9 | 🟡 Yellow | PWM Signal Input (from MCU) |
| **PWM-to-DAC - (GND L)** | Pin 6 | ⚫ Black | Logic Ground (MCU set to LOW) |
| **PWM-to-DAC - (VIN)** | GX12 Pin 5 | 🟡 Yellow | Power Supply (+) from Welder (10-12V) |
| **PWM-to-DAC - (GND In)** | MOSFET #2 (L) | ⚫ Black | Switched Ground (Galvanic Isolation) |
| **PWM-to-DAC - (Vo)** | GX12 Pin 7 | 🟢 Green | 0-10V Analog Output to welder |
| **LIPO-103048-1500 (+)** | JST BAT (LEFT) | 🔴 Thick Red | 3.7V Power (Check Polarity!) |
| **LIPO-103048-1500 (-)** | JST BAT (RIGHT)| ⚫ Thick Black | Ground |
| **Buzzer (Alarm)** | Pin A4 | 🟢 Green | Active Buzzer acoustic alarm |
| **GX12-5P Connector** | Male Plug | - | [See Pinout Table below] |
| **Power Button - Switch** | EN / GND | ⚫ Black | M10 Button (Latching) - Connect to EN / GND |
| **Power Button - LED (+)**| Pin A1 | 🔴 Red | Software controlled Status LED (Solid/Off) [Terminal A1 on Button] |
| **Power Button - LED (-)**| Pin A2 | ⚫ Black | Virtual Ground for LED (MCU set to LOW) [Terminal A2 on Button] |
| **Mode Button - Switch (+)**| Pin 0 (RX) | ⚪ White | Momentary Button for Menu/Modes (Internal Pullup) |
| **Mode Button - GND (-)**  | Pin 1 (TX) | ⚫ Black | Virtual Ground for Mode button (MCU set to LOW) |

### Aviation Connector Pinout (GX12-5P Male)

This connector links the Receiver unit to the welding machine's remote/pedal port. Note that while it uses a 5-pin physical layout, the pins are numbered 2, 3, 5, 6, and 7 based on the industrial standard for this specific machine.

| Pin (GX12) | Wire Color | Function | Receiver Connection |
| :---: | :---: | :--- | :--- |
| **2** | 🔴 Red | Trigger Switch (+) | **TLP222A #1 Pin 4** (Torch switch (+) · polarity not obligatory) |
| **3** | ⚫ Black | Trigger Switch (-) | **TLP222A #1 Pin 3** (Torch switch (-) · polarity not obligatory) |
| **5** | 🟡 Yellow | Remote Ref. (10-12V) | **PWM-to-DAC - (VIN)** (⚪ White in Thermal Ark) |
| **6** | 🟤 Brown | Remote GND (Min) | **TLP222A #2 Pin 4** (GND switch (+) · polarity not obligatory) |
| **7** | 🟢 Green | Remote Wiper (Sig) | **PWM-to-DAC - (Vo) (0-10V)** |

> [!IMPORTANT]
> **NO JUMPER REQUIRED FOR TLP222A:** Unlike the traditional discrete MOSFET modules, the Photocoupler TLP222A acts as a pure solid-state photorelay. Its outputs are completely isolated, dry, bidirectional contacts that don't share a common ground or power source, providing absolute galvanic isolation out-of-the-box.

### Component Specifications (Receiver)

| Component | Model / MPN | Key Specifications |
| :--- | :--- | :--- |
| **MCU + LoRa** | DIYmall LoRa32u4 RA-02 | ATmega32u4 · SX1278 433MHz · 3.3V logic · integrated LiPo charging |
| **OLED Display** | SSD1306 0.96" | 128x64px · I2C · 4 pins (VCC, GND, SCL, SDA) · 3.3V-5V |
| **Photocoupler** | TLP222A (x2) | Toshiba Photorelay · 4-pin DIP · 60V/500mA max · Solid-state switch · 2500 Vrms isolation · Link: https://es.aliexpress.com/item/1005009404744214.html |
| **DAC Converter** | PWM-to-DAC 0-10V | PWM Input · 0-5V or 0-10V Output · 12V-30V Power · Multi-turn Pot |
| **Battery** | LiPo 103048-1500mAh | 3.7V nom. · 10x30x48mm · integrated PCM |
| **Ext. Connector** | GX12-5P Male | Industrial aviation · 12mm · 5 contacts · Zinc/Nickel |

### Photocoupler Wiring Detail (TLP222A - 4-Pin DIP)

We use two independent **TLP222A** units (each is a single-channel photorelay in a 4-pin DIP package). They act as isolated, bidirectional solid-state switches.

#### Photocoupler #1: Torch Trigger (Relay Trigger - FULLY ISOLATED)
*   **Pin 1:** `LOGIC (+)` = **Pin A5** (MCU Trigger signal)
*   **Pin 2:** `GND (-)` = **Pin 10** (Virtual ground set to LOW by MCU)
*   **Pin 3:** `switch (-)` = **GX12 Pin 3 (⚫ Black)** (polarity not obligatory)
*   **Pin 4:** `switch (+)` = **GX12 Pin 2 (🔴 Red)** (polarity not obligatory)
*   *Note: When Pin A5 goes HIGH, the internal LED conducts, bridging Pin 3 and Pin 4 together to fire the torch trigger.*

#### Photocoupler #2: DAC Ground Isolation (FULLY ISOLATED)
*   **Pin 1:** `LOGIC (+)` = **Pin A3** (MCU DAC-ON signal)
*   **Pin 2:** `GND (-)` = **Pin 12** (Virtual ground set to LOW by MCU)
*   **Pin 3:** `switch (-)` = **PWM-to-DAC - (GND In)** (⚫ Black wire) (polarity not obligatory)
*   **Pin 4:** `switch (+)` = **GX12 Pin 6 (🟤 Brown)** (polarity not obligatory)
*   *Note: This switch disconnects the DAC module's ground when the pedal is at rest, ensuring a true 0.00V.*

---

### Ground Isolation Details (GND Isolation)

> [!IMPORTANT]
> The **TLP222A #2 (Pin A3)** does not cut the PWM signal; instead, it **physically connects/disconnects the PWM-to-DAC module's GND pin**. This ensures a **true 0.00V** at the analog output to the welder when the pedal is at rest.
