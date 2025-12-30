#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <Adafruit_SH110X.h>
#include <time.h>
#include <dht11.h>
#include "secrets.h" // Include your credentials
#include "language.h"  // Language file included.

// --- Network Configuration ---
const char* ssid = YOUR_SSID;
const char* password = YOUR_PASS;

// --- Global Objects ---
WiFiUDP udp;
unsigned int udpPort = 4210; // Port for UDP broadcasting
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

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  ArduinoOTA.setHostname("NetTime-Env-Server");
  ArduinoOTA.begin();

  configTime(gmtOffset_sec, 0, ntpServer);

  if (!display.begin(0x3C, true)) Serial.println("OLED Failed");
  display.setTextColor(SH110X_WHITE);

  setupWebServer();
  displayBrandLogo();
  delay(2000);
}

// Main logic for NetTime-Env-Server
void loop() {
  server.handleClient(); // Handle incoming web requests
  ArduinoOTA.handle();   // Handle Over-the-Air updates
  
  getLocalTime(&timeinfo); // Update time structure from system
  DHT.read(DHT11PIN);      // Read data from DHT11 sensor

  // PAGE SWITCHING LOGIC (Non-blocking)
  if (millis() - lastPageSwitch >= pageInterval) {
    currentPage = (currentPage + 1) % totalPages;
    lastPageSwitch = millis();
  }

  // REFRESH DISPLAY (Updates every loop to ensure smooth second counting)
  switch(currentPage) {
    case 0: displayDatePage(); break;
    case 1: displayTimePage(); break;
    case 2: displayWeatherPage(); break;
  }

  // UDP BROADCAST (Sends data to clients every 1 second)
  if (millis() - lastUdpSend >= 1000) {
    broadcastData();
    lastUdpSend = millis();
  }
}