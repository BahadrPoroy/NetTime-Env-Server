#ifndef NETWORK_MANAGER_H
#define NETWORK_MANAGER_H

#include "structs.h"
#include <ESP8266WiFi.h>
#include <FirebaseESP8266.h>
#include <ArduinoOTA.h>
#include <WiFiUdp.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include "secrets.h"

class NetworkManager {
private:
  FirebaseData firebaseData;
  FirebaseConfig config;
  FirebaseAuth auth;
  WiFiUDP udp;

  unsigned long lastWeatherCheck = 0;
  const unsigned long weatherInterval = 300000;  // 5 minutes (miliseconds)

  //Seperated ports for diffrent purposes
  const int OUTGOING_PORT = 4210;  //Sending port for Master
  const int INCOMING_PORT = 4211;  //Listening port for Master
                                   /**
   * @brief Fetches weather data from OpenWeather API using stream parsing to save RAM
   */
  void getWeatherData() {
    if (WiFi.status() != WL_CONNECTED) return;

    WiFiClient client;

    HTTPClient http;

    // Construct the URL using credentials from secrets.h
    String url = "http://api.openweathermap.org/data/3.0/onecall?lat=" + String(YOUR_LATITUDE) + "&lon=" + String(YOUR_LONGITUDE) + "&exclude=minutely,hourly,daily,alerts&units=metric&appid=" + String(YOUR_OPENWEATHER_API_KEY);

    if (http.begin(client, url)) {
      int httpCode = http.GET();

      if (httpCode == HTTP_CODE_OK) {
        // Define a filter to extract only the necessary fields from the large JSON response
        JsonDocument filter;
        filter["current"]["temp"] = true;
        filter["current"]["humidity"] = true;
        filter["current"]["weather"][0]["icon"] = true;
        filter["current"]["weather"][0]["description"] = true;

        // Use a DynamicJsonDocument to store the filtered result
        JsonDocument doc;
        // Parse the stream directly to avoid loading the entire string into memory
        DeserializationError error = deserializeJson(doc, http.getStream(), DeserializationOption::Filter(filter));

        if (!error) {
          currentWeather.temp = doc["current"]["temp"];
          currentWeather.humidity = doc["current"]["humidity"];
          currentWeather.icon = doc["current"]["weather"][0]["icon"].as<String>();
          currentWeather.description = doc["current"]["weather"][0]["description"].as<String>();
          currentWeather.updated = true;
        }
      } else {
        Serial.printf("[Weather] HTTP Error: %s\n", http.errorToString(httpCode).c_str());
      }
      http.end();
    }
  }

public:
  WeatherData currentWeather;

  void begin(TFT_eSPI &tft, DisplayManager &display) {
    WiFi.mode(WIFI_STA);
    WiFi.begin(YOUR_SSID, YOUR_PASS);

    Serial.print("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED) {
      display.updateLoadingAnimation(tft);
      delay(10);
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

    ArduinoOTA.onEnd([&tft]() {
      tft.fillScreen(TFT_BLACK);
      tft.loadFont(ATR16);
      tft.setTextColor(TFT_GREEN, TFT_BLACK);
      tft.setTextDatum(MC_DATUM);
      tft.setTextPadding(300);  // Ekran genişliğine yakın padding

      String text = String(TXT_UPDATE_SUCCESS);
      int newLinePos = text.indexOf('\n');

      if (newLinePos != -1) {
        // Eğer metinde \n varsa iki parça halinde yaz
        tft.drawString(text.substring(0, newLinePos), 160, 110);
        tft.drawString(text.substring(newLinePos + 1), 160, 140);
      } else if (tft.textWidth(text) > 300) {
        // Metin çok uzunsa ve \n yoksa, ortadan böl (veya sen dile göre manuel ayarla)
        tft.drawString(text, 160, 120);
      } else {
        tft.drawString(text, 160, 120);
      }

      tft.setTextPadding(0);
      tft.unloadFont();
    });

    ArduinoOTA.begin();
    udp.begin(INCOMING_PORT);
  }

  void updateFirebase(float temp, float hum, String time, String date, long ts, bool isFed, long lastFedTime) {
    Firebase.setFloat(firebaseData, "/NetTime/sicaklik", temp);
    Firebase.setFloat(firebaseData, "/NetTime/nem", hum);
    Firebase.setString(firebaseData, "/NetTime/son_guncelleme", time);
    Firebase.setString(firebaseData, "/NetTime/tarih", date);
    Firebase.setInt(firebaseData, "/NetTime/timestamp", ts);
    Firebase.setBool(firebaseData, "/NetTime/isFed", isFed);
    Firebase.setInt(firebaseData, "/NetTime/lastFedTime", lastFedTime);
  }

  void readFirebase(bool &isFed, long &lastFedTime) {
    if (Firebase.getBool(firebaseData, "/NetTime/isFed")) {
      isFed = firebaseData.boolData();
    }
    if (Firebase.getInt(firebaseData, "/NetTime/lastFedTime")) {
      lastFedTime = firebaseData.intData();
    }
  }

  void broadcastUDP(String message) {
    IPAddress broadcastIP = WiFi.localIP();
    broadcastIP[3] = 255;
    udp.beginPacket(broadcastIP, OUTGOING_PORT);  //Updated
    udp.write(message.c_str());
    udp.endPacket();
  }

  void handleOTA() {
    ArduinoOTA.handle();
  }

  int getSignalLevel() {
    long rssi = WiFi.RSSI();
    if (rssi > -55) return 4;        // Excellent
    else if (rssi > -70) return 3;   // Good
    else if (rssi > -85) return 2;   // Fair
    else if (rssi > -100) return 1;  // Weak
    return 0;                        // No signal
  }

  void handleFeederNetwork(String &currentFedState, bool &isFed, long &lastFedTime, long currentTimestamp) {
    int packetSize = udp.parsePacket();

    if (packetSize) {
      char buf[32];
      int len = udp.read(buf, 31);
      buf[len] = 0;
      String resp = String(buf);
      resp.trim();

      currentFedState = resp;

      if (resp == "SUCCESS") {
        isFed = true;
        lastFedTime = currentTimestamp;
      } else if (resp == "SYSTEM_READY_IDLE" || resp == "IDLE") {
        currentFedState = "IDLE";
      }
    }
  }

  /**
   * @brief Handles periodic weather updates based on the defined interval
   */
  void handleOpenWeather() {
    // Initial call or periodic check
    if (millis() - lastWeatherCheck >= weatherInterval || lastWeatherCheck == 0) {
      lastWeatherCheck = millis();
      getWeatherData();
    }
  }
};
#endif