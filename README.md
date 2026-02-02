# 🕒 NetTime-Env-Server V2.3.0-alpha 🌐
![C++](https://img.shields.io/badge/C++-00599C?style=flat-square&logo=c%2B%2B&logoColor=white)
![Firebase](https://img.shields.io/badge/Firebase-ffca28?style=flat-square&logo=firebase&logoColor=black)
![ESP8266](https://img.shields.io/badge/ESP8266-414141?style=flat-square&logo=espressif&logoColor=white)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg?style=flat-square)](https://opensource.org/licenses/MIT)
### "The NetTime OS - Premium TFT Edition"

## ⚠️ Note: Version 2.3.0 is currently in Alpha. While UI rendering issues (padding leaks) have been resolved, please report any bugs via GitHub Issues. ##

## 📸 Media

<p align="center">
  <img src="Assets/V1.2.0.gif" width="600" title="NetTime Server Live Dashboard">
</p>
<p align="center">
  <video src="Assets/Project_Showcase.mp4" width="400" autoplay muted loop>
  </video>
</p>

**NetTime-Env-Server** is a centralized environmental hub designed to provide synchronized time and climate data. It is the official successor to my previous [Date-Time-Temperature](https://github.com/BahadrPoroy/Date-Time-Tempreture) project.

While the original project was a standalone clock, this version transforms the device into a **Cloud-Connected Data Master**, capable of serving multiple clients and a real-time web interface simultaneously via Firebase.

## 🚀 What's New in v2.3.0-alpha

- **Enhanced UDP Port Configuration:** Switched to separate ports for RX and TX. This separation minimizes potential collisions and improves connection stability.

- **Evolution of UI/UX Framework:** Transitioned from a legacy list-based navigation to a modern, icon-driven Desktop interface. This update introduces a dedicated desktop home screen and an intuitive navigation engine for enhanced user accessibility.

- **Persistent Feeding Logs & Universal State Template:** Automated synchronization of device states with Firebase, providing cross-device persistence and a scalable architecture for future IoT applications.

## 🚧 Under Construction: The Home and Settings icons represent upcoming features. Navigation to these pages is currently restricted as we finalize the backend integration for these specific modules. ##

## 📂 Project Structure

The project has been refactored from a single-file script into a modular, header-based library system to improve scalability and maintainability.

* **DisplayManager.h**: Manages the "NetTime OS" graphical interface, utilizing `TFT_eSPI`. It handles the Windows-style Taskbar, Start Menu logic, and UI animations.
* **NetworkManager.h**: Handles all wireless communications including WiFi station mode, Firebase integration, UDP Broadcasts, and ArduinoOTA update callbacks.
* **TimeManager.h**: Manages NTP server synchronization and provides high-precision time formatting (`HH:MM:SS`) and date strings.
* **TouchManager.h**: Processes touch panel inputs and maps them to screen coordinates using hardware-specific calibration data.
* **config.h**: The Master Configuration file. Contains all pin assignments (HAL) and global system constants.
* **myFonts.h**: All font headers are organized within a dedicated Fonts folder and consolidated into a single myFonts.h header. This centralized structure simplifies font library management and improves code readability. 

## 🛠️ Hardware Requirements

- **MCU:** ESP8266 (NodeMCU or Wemos D1 Mini)
- **Sensor:** DHT11 / DHT22 (Temp & Humidity)
- **Display:** 2.8" ILI9341 TFT display (SPI)
- **Storage:** MicroSD Card (for assets and fonts)

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
- **5:** Upload the `assets/` folder to your SD card (Ensure the file names match the definitions in DisplayManager.h).
