# NetTime-Env-Server V1.0 🌐

<p align="center">
  <img src="Assets/dashboard-preview.gif" width="600" title="NetTime Server Live Dashboard">
</p>

**NetTime-Env-Server** is a centralized network hub designed to provide synchronized time and environmental data across a local network. It is the official successor to my previous [Date-Time-Tempreture](https://github.com/BahadrPoroy/Date-Time-Tempreture) project.

While the original project was a standalone clock, this version transforms the device into a **Data Master**, capable of serving multiple clients and a web interface simultaneously.

## 🔄 The Evolution: What’s New?

This project was rebuilt from the ground up to support a server-client architecture.

### ✨ Major Upgrades
- **UDP Broadcasting:** Automatically pushes `Time|Temp` data to all devices on the network every second.
- **Web Dashboard:** A real-time, dark-themed HTML/CSS interface with AJAX/JSON integration (no page refresh needed).
- **OTA Updates:** Support for wireless firmware updates over the air.
- **Improved Display Engine:** Optimized logic for smooth, real-time second counting on OLED.
- **Modular Code:** Separated into `Clock`, `Date`, and `Weather` modules for better readability.

### 🗑️ Simplified Logic
- Replaced the monolithic script with a modular structure.
- Removed legacy UI elements to focus on network performance and data accuracy.

## 🛠️ Hardware Requirements
- **MCU:** ESP8266 (NodeMCU or Wemos D1 Mini)
- **Sensor:** DHT11 (Temp & Humidity)
- **Display:** 1.3" SH1106 OLED (I2C)

## 📡 Protocol Info
The server broadcasts a UDP packet on port **4210** in the following format:
`HH:MM|TEMP` (Example: `20:45|26`)

## 📦 Getting Started
1. Open `secrets.h` and enter your WiFi credentials in `YOUR_SSID` and `YOUR_PASS`.
2. Ensure you have the required libraries: `Adafruit_SH110X`, `dht11`, `ESP8266WiFi`.
3. Flash the code and monitor the IP address via Serial Monitor to access the web dashboard.