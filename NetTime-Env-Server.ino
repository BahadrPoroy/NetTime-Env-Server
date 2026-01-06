#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <Adafruit_SH110X.h>
#include <time.h>
#include <dht11.h>
#include "secrets.h"                   // Include your credentials
#include "language.h"                  // Language file included.
#include <Fonts/FreeSansBold9pt7b.h>   // 9pt font library
#include <Fonts/FreeSansBold12pt7b.h>  // 12pt font library
#include <FirebaseESP8266.h>


#define FIREBASE_HOST YOUR_URL
#define FIREBASE_AUTH YOUR_DATABASE_SECRET_KEY

FirebaseConfig config;
FirebaseAuth auth;

FirebaseData firebaseData;

unsigned long lastFirebaseUpdate = 0;
const long updateInterval = 10000;  // Interval for updating Firebase (10 seconds)

// --- Network Configuration ---
const char* ssid = YOUR_SSID;
const char* password = YOUR_PASS;
const char* otaPass = YOUR_OTA_PASS;

// --- Global Objects ---
WiFiUDP udp;
unsigned int udpPort = 4210;  // Port for UDP broadcasting
ESP8266WebServer server(80);

dht11 DHT;

// --- Time Settings (GMT+3) ---
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 3 * 3600;
struct tm timeinfo;

// --- Display Settings ---
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define DHT11PIN 14
Adafruit_SH1106G display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// --- Timing & Logic ---
int currentPage = 0;
const int totalPages = 3;
unsigned long lastPageSwitch = 0;
const unsigned long pageInterval = 2500;
unsigned long lastUdpSend = 0;

void setupOTA() {
  ArduinoOTA.setHostname("NetTime-Env-Server");
  ArduinoOTA.setPassword(otaPass);

  ArduinoOTA.onStart([]() {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SH110X_WHITE);
    display.setCursor(10, 10);
    display.println("Guncelleniyor...");
    display.display();
  });

  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    int yuzde = progress / (total / 100);

    display.clearDisplay();
    display.setFont(&FreeSansBold9pt7b);
    display.setTextColor(SH110X_WHITE);

    display.setCursor(5, 25);
    display.print("Guncelleniyor");

    display.setCursor(40, 40);
    display.print("%");
    display.print(yuzde);

    display.setFont();
    display.drawRect(5, 52, 118, 10, SH110X_WHITE);
    display.fillRect(5, 52, map(yuzde, 0, 100, 0, 118), 10, SH110X_WHITE);

    display.display();
  });

  ArduinoOTA.onEnd([]() {
    display.clearDisplay();
    display.setTextColor(SH110X_WHITE);

    display.setFont(&FreeSansBold9pt7b);
    display.setTextSize(1);

    display.setCursor(10, 25);
    display.print("BASARILI!");

    display.setFont();
    display.setTextSize(1);

    display.setCursor(10, 45);
    display.print("Yeniden basliyor...");

    display.display();
  });

  ArduinoOTA.onError([](ota_error_t error) {
    display.clearDisplay();
    display.setCursor(10, 10);
    display.print("ERROR: ");
    display.println(error);
    display.display();
  });

  ArduinoOTA.begin();
}

void setup() {
  Serial.begin(115200);

  if (!display.begin(0x3C, true)) Serial.println("OLED Failed");
  display.setTextColor(SH110X_WHITE);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  setupOTA();

  configTime(gmtOffset_sec, 0, ntpServer);

  setupWebServer();

  config.host = YOUR_URL;
  config.signer.tokens.legacy_token = YOUR_DATABASE_SECRET_KEY;
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  displayBrandLogo();
  delay(2000);
}

// Main logic for NetTime-Env-Server
void loop() {
  server.handleClient();  // Handle incoming web requests
  ArduinoOTA.handle();    // Handle Over-the-Air updates

  getLocalTime(&timeinfo);  // Update time structure from system
  DHT.read(DHT11PIN);       // Read data from DHT11 sensor

  // PAGE SWITCHING LOGIC (Non-blocking)
  if (millis() - lastPageSwitch >= pageInterval) {
    currentPage = (currentPage + 1) % totalPages;
    lastPageSwitch = millis();
  }

  // REFRESH DISPLAY (Updates every loop to ensure smooth second counting)
  switch (currentPage) {
    case 0: displayDatePage(); break;
    case 1: displayTimePage(); break;
    case 2: displayWeatherPage(); break;
  }

  // UDP BROADCAST (Sends data to clients every 1 second)
  if (millis() - lastUdpSend >= 1000) {
    broadcastData();
    lastUdpSend = millis();
  }

  // FIREBASE Update (Sends data to firebase every 10 seconds)
  if (millis() - lastFirebaseUpdate >= updateInterval) {
    updateFirebase();
    lastFirebaseUpdate = millis();
  }
}