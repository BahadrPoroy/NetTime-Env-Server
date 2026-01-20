#ifndef NETWORK_MANAGER_H
#define NETWORK_MANAGER_H

#include <ESP8266WiFi.h>
#include <FirebaseESP8266.h>
#include <ArduinoOTA.h>
#include <WiFiUdp.h>
#include "secrets.h"

class NetworkManager {
private:
  FirebaseData firebaseData;
  FirebaseConfig config;
  FirebaseAuth auth;
  WiFiUDP udp;
  const int udpPort = 4210;

public:
  void begin(TFT_eSPI &tft, DisplayManager &display) {
    WiFi.mode(WIFI_STA);
    WiFi.begin(YOUR_SSID, YOUR_PASS);

    Serial.print("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
    Serial.println("\nWiFi Connected");

    config.host = YOUR_URL;
    config.signer.tokens.legacy_token = YOUR_DATABASE_SECRET_KEY;
    Firebase.begin(&config, &auth);
    Firebase.reconnectWiFi(true);

    // OTA Callbacks linked to DisplayManager
    ArduinoOTA.setHostname("NetTime-Env-Server");
    ArduinoOTA.setPassword(YOUR_OTA_PASS);

    static int lastPercentage = -1;  // To track progress and prevent flicker

    ArduinoOTA.onStart([&tft, &display]() {
      lastPercentage = -1;  // Reset tracker
      display.showUpdateScreen(tft, 0);
    });

    ArduinoOTA.onProgress([&tft, &display](unsigned int progress, unsigned int total) {
      int percentage = (progress / (total / 100));

      // ANTI-FLICKER LOGIC: Only update if percentage has actually changed
      if (percentage != lastPercentage) {
        lastPercentage = percentage;
        display.showUpdateScreen(tft, percentage);
      }
    });

    ArduinoOTA.onEnd([&tft, &display]() {
      tft.fillScreen(TFT_GREEN);
      tft.setTextColor(TFT_BLACK);
      tft.drawString("SUCCESS! REBOOTING...", 160, 120);
    });

    ArduinoOTA.begin();
    udp.begin(udpPort);
  }

  void
  updateFirebase(float temp, float hum, String time, String date, long ts) {
    Firebase.setFloat(firebaseData, "/NetTime/sicaklik", temp);
    Firebase.setFloat(firebaseData, "/NetTime/nem", hum);
    Firebase.setString(firebaseData, "/NetTime/son_guncelleme", time);
    Firebase.setString(firebaseData, "/NetTime/tarih", date);
    Firebase.setInt(firebaseData, "/NetTime/timestamp", ts);
  }

  void broadcastUDP(String message) {
    IPAddress broadcastIP = WiFi.localIP();
    broadcastIP[3] = 255;
    udp.beginPacket(broadcastIP, udpPort);
    udp.write(message.c_str());
    udp.endPacket();
  }

  void handleOTA() {
    ArduinoOTA.handle();
  }

  int getSignalLevel() {
    long rssi = WiFi.RSSI();
    if (rssi > -55) return 4;      // Excellent
    else if (rssi > -70) return 3; // Good
    else if (rssi > -85) return 2; // Fair
    else if (rssi > -100) return 1;// Weak
    return 0;                      // No signal
}

};
#endif