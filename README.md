# NetTime-Env-Server V1.1 🌐

<p align="center">
  <img src="Assets/dashboard-preview.gif" width="600" title="NetTime Server Live Dashboard">
</p>

**NetTime-Env-Server** is a centralized environmental hub designed to provide synchronized time and climate data. It is the official successor to my previous [Date-Time-Tempreture](https://github.com/BahadrPoroy/Date-Time-Tempreture) project.

While the original project was a standalone clock, this version transforms the device into a **Cloud-Connected Data Master**, capable of serving multiple clients and a real-time web interface simultaneously via Firebase.

## 🔄 The Evolution: What’s New in V1.1.0?

The project has been upgraded to support a cloud-native architecture with significant UI/UX improvements.

### ✨ Major Upgrades
- **Firebase Integration:** Real-time data synchronization across the globe, moving beyond just local network broadcasting.
- **Smart Theme Engine:** Automatic Light/Dark mode detection based on user's system preferences with manual toggle persistence.
- **Digital Typography:** Integrated 'Orbitron' font for a modern, industrial dashboard aesthetic.
- **Multilingual Support:** Dynamic language switching (EN/TR) without page reloads.
- **Google Analytics (GA4):** Integrated tracking to monitor dashboard traffic and engagement.
- **Security:** Enhanced database security with authorized domain restrictions and Firebase rules.

### 🗑️ Optimized Logic
- **Modular Frontend:** Separated CSS and JavaScript into dedicated files for better maintainability.
- **Commented Codebase:** All variables and logic are now documented in English for global accessibility.

## 🛠️ Hardware Requirements
- **MCU:** ESP8266 (NodeMCU or Wemos D1 Mini)
- **Sensor:** DHT11 / DHT22 (Temp & Humidity)
- **Display:** 1.3" SH1106 OLED (I2C)

## 📡 Technology Stack
- **Backend:** Firebase Realtime Database.
- **Frontend:** HTML5, CSS3 (Custom Variables), Vanilla JavaScript.
- **Tracking:** Google Analytics GA4.

## 📦 Getting Started
1. Clone the repository to your local machine.
2. Replace the `firebaseConfig` object in `index.html` with your own project credentials from the Firebase Console.
3. Ensure your NodeMCU is configured to push data to the same Firebase paths (`/NetTime/sicaklik`, etc.).
4. Open `index.html` via any modern browser or host it using GitHub Pages.