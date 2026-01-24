# 🕒 NetTime-Env-Server V2.1.0-beta 🌐
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)  
### "The NetTime OS - Premium TFT Edition"


<p align="center">
  <img src="Assets/V1.2.0.gif" width="600" title="NetTime Server Live Dashboard">
</p>

**NetTime-Env-Server** is a centralized environmental hub designed to provide synchronized time and climate data. It is the official successor to my previous [Date-Time-Tempreture](https://github.com/BahadrPoroy/Date-Time-Tempreture) project.

While the original project was a standalone clock, this version transforms the device into a **Cloud-Connected Data Master**, capable of serving multiple clients and a real-time web interface simultaneously via Firebase.

## 🚀 What's New in v2.1.0-beta

- **System Console:** Advanced real-time monitoring of Heap Memory, CPU Frequency, Flash storage, and Network status.
- **Safe Shutdown Protocol:** A professional "Clean Shutdown" sequence that safely unmounts the SD Card and deactivates WiFi/SPI radios before rebooting.
- **Multi-Language Core:** Dynamic localization support (EN/TR) managed via a single header file and extended.
- **Visual OTA:** Live progress bar on the screen during wireless updates.

## 📂 Project Structure

The project has been refactored from a single-file script into a modular, header-based library system to improve scalability and maintainability.

* **DisplayManager.h**: Manages the "NetTime OS" graphical interface, utilizing `TFT_eSPI`. It handles the Windows-style Taskbar, Start Menu logic, and UI animations.
* **NetworkManager.h**: Handles all wireless communications including WiFi station mode, Firebase integration, UDP Broadcasts, and ArduinoOTA update callbacks.
* **TimeManager.h**: Manages NTP server synchronization and provides high-precision time formatting (`HH:MM:SS`) and date strings.
* **TouchManager.h**: Processes touch panel inputs and maps them to screen coordinates using hardware-specific calibration data.
* **config.h**: The Master Configuration file. Contains all pin assignments (HAL) and global system constants.

## 🛠️ Hardware Requirements

- **MCU:** ESP8266 (NodeMCU or Wemos D1 Mini)
- **Sensor:** DHT11 / DHT22 (Temp & Humidity)
- **Display:** 2.8" ILI9341 TFT display (SPI)

## 📡 Technology Stack
- **Firmware:** Arduino IDE (C++), ArduinoJson, NTPClient.
- **Backend:** Firebase Realtime Database.
- **Frontend:** HTML5, CSS3 (Custom Variables), Vanilla JavaScript.
- **Tracking:** Google Analytics GA4.

## 🌐 Setup & Installation
- **1:** Ensure your hardware matches the wiring in `config.h`.
- **2:** Create a `secrets.h` file with your WiFi, Firebase, and OTA credentials.
- **3:** Use the **Arduino IDE** or **PlatformIO** with the `TFT_eSPI` library installed.
- **4:** Set your display driver to **ILI9341** in the library's user setup.
