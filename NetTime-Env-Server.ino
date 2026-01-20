#include <TFT_eSPI.h>
#include <dht11.h>
#include <SD.h>
#include "config.h"
#include "DisplayManager.h"
#include "TouchManager.h"
#include "NetworkManager.h"
#include "TimeManager.h"

// Global instances
TFT_eSPI tft = TFT_eSPI();
dht11 DHT;
DisplayManager displayBox;
TouchManager touchBox;
NetworkManager netBox;
TimeManager timeBox;

unsigned long lastClockUpdate = 0;
unsigned long lastSensorUpdate = 0;
unsigned long lastFirebaseSync = 0;
unsigned long lastUIUpdate = 0;
int currentPage = -1, activePage;

void setup() {
  Serial.begin(115200);
  pinMode(TFT_LED, OUTPUT);
  digitalWrite(TFT_LED, HIGH);
  tft.setFreeFont();

  displayBox.init(tft);
  netBox.begin(tft, displayBox);
  timeBox.begin();
  touchBox.begin(tft);
  displayBox.drawWelcomeScreen(tft);
  displayBox.drawStaticUI(tft);

  // Get current signal level
  int wifiLevel = netBox.getSignalLevel();

  // Update Clock and Wifi Icon
  displayBox.updateClock(tft, timeBox.getFormattedTime(), timeBox.getFormattedDate());
  displayBox.drawWifiIcon(tft, wifiLevel);
  lastUIUpdate = millis();
}

void loop() {
  netBox.handleOTA();

  // 1. Clock Update (Every 1 second) - Smooth
  if (millis() - lastClockUpdate >= 1000) {
    lastClockUpdate = millis();
    displayBox.updateClock(tft, timeBox.getFormattedTime(), timeBox.getFormattedDate());
  }

  if (millis() - lastSensorUpdate >= 1000) {
    lastSensorUpdate = millis();
    DHT.read(DHTPIN);
  }

  // 3. Firebase Update (Every 20 seconds) - Optimized
  if (millis() - lastFirebaseSync >= 20000) {
    lastFirebaseSync = millis();
    netBox.updateFirebase(
      (float)DHT.temperature, (float)DHT.humidity,
      timeBox.getFormattedTime(), timeBox.getFormattedDate(), timeBox.getTimestamp());
  }

  uint16_t x, y;
  if (touchBox.isPressed(tft, x, y)) {
    // 1. Check if Start Icon (Bottom-Left) is pressed
    // Area: x[0-50], y[210-240]
    if (x < 50 && y > 205) {
      if (!displayBox.isMenuOpen) {
        displayBox.drawStartMenu(tft);
      } else {
        displayBox.hideStartMenu(tft);
      }
      delay(300);  // Debounce
    }
    // 2. Check Taskbar Clock Area (Right side)
    if (x > 240 && y > 205) {
      if (!displayBox.isClockExpanded) {

        String currentSsid = WiFi.SSID();
        displayBox.drawExpandedClock(tft, timeBox.getFormattedTime(), timeBox.getFormattedDate(), timeBox.getDayName(), currentSsid, true);
      } else {
        displayBox.hideExpandedClock(tft);
      }
      delay(300);
    }

    // 3. Check if RESTART button is pressed (When menu is open)
    if (displayBox.isMenuOpen) {
      // Area: x[5-100], y[185-210]
      if (x > 5 && x < 100 && y > 185 && y < 210) {
        tft.fillScreen(TFT_BLACK);
        tft.setTextColor(TFT_RED);
        tft.setTextDatum(MC_DATUM);
        tft.drawString("REBOOTING...", 160, 120, 2);
        delay(1000);
        ESP.restart();  // Software Reset
      }
    }
  }
  if (millis() - lastUIUpdate >= 1000) {
    lastUIUpdate = millis();

    // Get current signal level
    int wifiLevel = netBox.getSignalLevel();
    String currentSSID = WiFi.SSID();

    displayBox.updateWeather(tft, (float)DHT.temperature, (float)DHT.humidity);
    // Update Clock and Wifi Icon
    displayBox.updateClock(tft, timeBox.getFormattedTime(), timeBox.getFormattedDate());
    displayBox.drawWifiIcon(tft, wifiLevel);

    // If expanded clock is open, maybe show signal there too
    if (displayBox.isClockExpanded) {
      displayBox.drawExpandedClock(tft, timeBox.getFormattedTime(), timeBox.getFormattedDate(), timeBox.getDayName(), currentSSID);
    }
  }

  if (currentPage != activePage) {
    if (activePage == 0) displayBox.drawHeader(tft, WEATHER_TITLE, 0x0063, 0xffff);
    else if (activePage == 1) displayBox.drawHeader(tft, SETTINGS_TITLE, 0x18c3, 0xffff);
    else if (activePage == 2) displayBox.drawHeader(tft, ABOUT_TITLE, 0x18c3, 0xffff);

    currentPage = activePage;
  }
}