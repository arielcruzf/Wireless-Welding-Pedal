# ⚡ Wireless Welding DIY Pedal (LoRa32u4)

A professional-grade, long-range wireless TIG welding pedal based on LoRa technology. This project eliminates cables in the workshop while maintaining zero-latency and high precision.

## 🚀 Key Features
- **Zero Latency:** High-speed LoRa communication (433MHz).
- **Precision Sensing:** VL53L4CD Time-of-Flight (ToF) laser sensor for exact distance-to-current mapping.
- **Ultra-Low Power:** optimized deep sleep logic for months of battery life.
- **Smart Receiver:** Industrial-grade signal conversion (0-10V) with MOSFET isolation.
- **Web Dashboard:** Real-time telemetry and calibration interface (Next.js).

## 📁 Repository Structure
- `FIRMWARE/`: Source code for the Pedal (Transmitter) and the Machine Bridge (Receiver).
- `HARDWARE/`: 3D models, schematics, and wiring diagrams.
- `WEB/`: Next.js dashboard for monitoring and calibration.
- `DOCS/`: Technical documentation, logic flows, and roadmap.
- `MEDIA/`: Project images and video demonstrations.

## 🛠️ Tech Stack
- **MCU:** ATmega32u4 (LoRa32u4 RA-02)
- **Radio:** SX1278 (LoRa)
- **Sensor:** VL53L4CD (ToF)
- **Web:** Next.js + TailwindCSS + Gemini AI Integration

## 🔧 Setup
1. Clone the repository.
2. Upload the `FIRMWARE/transmitter` code to the pedal unit.
3. Upload the `FIRMWARE/receiver` code to the receiver unit.
4. Run `npm install` in the `WEB/` directory to start the dashboard.

---
*Created by ARICF - Professional DIY Welding Tools*
