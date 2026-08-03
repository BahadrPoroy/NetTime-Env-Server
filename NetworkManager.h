#ifndef NETWORK_MANAGER_H
#define NETWORK_MANAGER_H

#include "structs.h"
#include <WiFi.h>
#include <FirebaseESP8266.h>
#include <ArduinoOTA.h>
#include <WiFiUdp.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include "secrets.h"

class NetBoxManager {
private:
  FirebaseData firebaseData;
  FirebaseConfig config;
  FirebaseAuth auth;
  WiFiUDP udp;

  unsigned long lastWeatherCheck = 0;
  const unsigned long weatherInterval = 300000;  // 5 minutes (milliseconds)

  // Separated ports for different purposes
  const int OUTGOING_PORT = 4210;  // Sending port for Master
  const int INCOMING_PORT = 4211;  // Listening port for Master

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
        filter["current"]["sunrise"] = true;
        filter["current"]["sunset"] = true;

        // Use a JsonDocument to store the filtered result
        JsonDocument doc;
        // Parse the stream directly to avoid loading the entire string into memory
        DeserializationError error = deserializeJson(doc, http.getStream(), DeserializationOption::Filter(filter));

        if (!error) {
          currentWeather.temp = doc["current"]["temp"];
          currentWeather.humidity = doc["current"]["humidity"];
          currentWeather.icon = doc["current"]["weather"][0]["icon"].as<String>();
          currentWeather.description = doc["current"]["weather"][0]["description"].as<String>();
          currentWeather.sunrise = doc["current"]["sunrise"];
          currentWeather.sunset = doc["current"]["sunset"];
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

  void begin() {
    WiFi.mode(WIFI_STA);
    WiFi.begin(YOUR_SSID, YOUR_PASS);

    Serial.print("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED) {
      Serial.print(".");
      delay(500);
    }
    Serial.println("\nWiFi Connected");

    config.host = YOUR_URL;
    config.signer.tokens.legacy_token = YOUR_DATABASE_SECRET_KEY;
    Firebase.begin(&config, &auth);
    Firebase.reconnectWiFi(true);

    // OTA Callbacks
    ArduinoOTA.setHostname("NetTime-Env-Server");
    ArduinoOTA.setPassword(YOUR_OTA_PASS);

    static int lastPercentage = -1;  // To track progress and prevent spamming serial

    ArduinoOTA.onStart([]() {
      lastPercentage = -1;
      Serial.println("[OTA] Güncelleme Başlatıldı...");
    });

    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
      int percentage = (progress / (total / 100));

      // ANTI-FLICKER LOGIC: Only update if percentage has actually changed
      if (percentage != lastPercentage) {
        lastPercentage = percentage;
        Serial.printf("[OTA] İlerleme: %d%%\r\n", percentage);
      }
    });

    ArduinoOTA.onEnd([]() {
      Serial.println("\n[OTA] Güncelleme Başarıyla Tamamlandı!");
    });

    ArduinoOTA.onError([](ota_error_t error) {
      Serial.printf("[OTA] Hata [%u]: ", error);
      if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
      else if (error == OTA_END_ERROR) Serial.println("End Failed");
    });

    ArduinoOTA.begin();
    udp.begin(INCOMING_PORT);
  }

  void updateFirebase(float temp, float hum, String time, String date, long ts, volatile bool isFed, volatile long lastFedTime) {
    Firebase.setFloat(firebaseData, "/NetTime/sicaklik", temp);
    Firebase.setFloat(firebaseData, "/NetTime/nem", hum);
    Firebase.setString(firebaseData, "/NetTime/son_guncelleme", time);
    Firebase.setString(firebaseData, "/NetTime/tarih", date);
    Firebase.setInt(firebaseData, "/NetTime/timestamp", ts);
    Firebase.setBool(firebaseData, "/NetTime/isFed", isFed);
    Firebase.setInt(firebaseData, "/NetTime/lastFedTime", lastFedTime);
  }

  //--- Firebase Read for Settings ---
  void readSettings(SettingsData &settings) {
    if (Firebase.getJSON(firebaseData, "/Settings")) {
      FirebaseJson settingsJson = firebaseData.jsonObject();
      FirebaseJsonData data;

      if (settingsJson.get(data, "feederStart"))
        settings.feederStart = data.intValue;
      if (settingsJson.get(data, "feederEnd"))
        settings.feederEnd = data.intValue;
      if (settingsJson.get(data, "dayBright"))
        settings.dayBright = data.intValue;
      if (settingsJson.get(data, "manBright"))
        settings.manBright = data.intValue;
      if (settingsJson.get(data, "nightBright"))
        settings.nightBright = data.intValue;
      if (settingsJson.get(data, "isAdaptive"))
        settings.isAdaptive = data.boolValue;
      if (settingsJson.get(data, "language"))
        settings.language = data.intValue;
    }
  }

  // --- Firebase Update for Settings ---
  void updateSetting(String key, int value) {
    Firebase.setInt(firebaseData, "/Settings/" + key, value);
  }

  // Overload: Bool değerler için (isAdaptive)
  void updateSetting(String key, bool value) {
    Firebase.setBool(firebaseData, "/Settings/" + key, value);
  }

  void readFirebase(volatile bool &isFed, volatile long &lastFedTime) {
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
    udp.beginPacket(broadcastIP, OUTGOING_PORT);
    udp.write((const uint8_t *)message.c_str(), message.length());
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

  // Web Arayüzünden gelen tetikleyiciyi kontrol eder
  void checkRemoteFeedTrigger() {
    if (WiFi.status() != WL_CONNECTED) return;

    // Firebase üzerindeki /NetTime/triggerFeed yolunu kontrol et
    if (Firebase.getBool(firebaseData, "/NetTime/triggerFeed")) {
      if (firebaseData.dataType() == "boolean" && firebaseData.boolData() == true) {
        Serial.println("[REMOTE FEED] Web arayüzünden yemleme emri alındı! UDP yayınlanıyor...");

        // Yerel ağa UDP paketi yayınla
        broadcastUDP("FEED_NOW");

        // Tetikleyiciyi sıfırla (tekrar tetiklenebilmesi için)
        Firebase.setBool(firebaseData, "/NetTime/triggerFeed", false);
      }
    }
    if (Firebase.getBool(firebaseData, "/NetTime/triggerReset")) {
      if (firebaseData.dataType() == "boolean" && firebaseData.boolData() == true) {
        Serial.println("[REMOTE RESTART] Web arayüzünden restart emri alındı! UDP yayınlanıyor...");

        // Yerel ağa UDP paketi yayınla
        broadcastUDP("RESTART");

        // Tetikleyiciyi sıfırla (tekrar tetiklenebilmesi için)
        Firebase.setBool(firebaseData, "/NetTime/triggerReset", false);
      }
    }
  }

  void handleFeederNetwork(String &currentFedState, volatile bool &isFed, volatile long &lastFedTime, long currentTimestamp) {
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
      } else if (resp == "IDLE") {
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