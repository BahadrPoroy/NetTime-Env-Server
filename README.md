# 🕒 NetTime-Env-Server V1.2.0 🌐

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

<p align="center">
  <img src="Assets/V1.2.0.gif" width="600" title="NetTime Server Live Dashboard">
</p>

**NetTime-Env-Server** is a centralized environmental hub designed to provide synchronized time and climate data. It is the official successor to my previous [Date-Time-Tempreture](https://github.com/BahadrPoroy/Date-Time-Tempreture) project.

While the original project was a standalone clock, this version transforms the device into a **Cloud-Connected Data Master**, capable of serving multiple clients and a real-time web interface simultaneously via Firebase.

## 🚀 What's New in v1.2.0

- **High-Precision Sync:** Integrated a new driftCorrection algorithm to eliminate time lag between the browser and MCU.
- **Fluid UI:** Upgraded clock updates to requestAnimationFrame, achieving a stutter-free 60 FPS visual experience.
- **OLED Optimization:** Redesigned the onboard OLED layout to resolve overlapping issues between the date string and Wi-Fi status icons.
- **Performance:** Refactored communication.ino by removing redundant local server functions to optimize memory and CPU cycles.

### 🗑️ Optimized Logic

- **Modular Frontend:** Separated CSS and JavaScript into dedicated files for better maintainability.
- **Commented Codebase:** All variables and logic are now documented in English for global accessibility.

## 🛠️ Hardware Requirements

- **MCU:** ESP8266 (NodeMCU or Wemos D1 Mini)
- **Sensor:** DHT11 / DHT22 (Temp & Humidity)
- **Display:** 1.3" SH1106 OLED (I2C)

## 📡 Technology Stack
- **Firmware:** Arduino IDE (C++), ArduinoJson, NTPClient.
- **Backend:** Firebase Realtime Database.
- **Frontend:** HTML5, CSS3 (Custom Variables), Vanilla JavaScript.
- **Tracking:** Google Analytics GA4.

## 🌐 Setup & Installation
- **1:** Flash your NodeMCU with the NetTime-Env-Server.ino using Arduino IDE.
- **2:** Update your Firebase credentials in both script.js and the .ino source files.
- **3:** Open the web dashboard in any modern browser to start monitoring.
