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

enum Page {
  NONE,          //Initial state
  WEATHER_PAGE,  // Main dashboard with clock and sensors
  SYSTEM_PAGE    // System Properties and device info
};

Page currentPage = NONE;


void setup() {
  Serial.begin(115200);
  pinMode(TFT_LED, OUTPUT);
  digitalWrite(TFT_LED, HIGH);

  if (!SD.begin(SD_CS, SPI_HALF_SPEED)) {
    Serial.println("SD Card Error!");
  }

  displayBox.init(tft);

  // Show logo immediately
  displayBox.drawStaticSplash(tft);

  // Connect WiFi while animating
  timeBox.begin();
  netBox.begin(tft, displayBox);

  tft.fillScreen(TFT_BLACK);
  displayBox.drawTaskbar(tft);
  switchPage(WEATHER_PAGE);
}

void loop() {
  netBox.handleOTA();

  // Unified UI and Sensor Update (Every 1 second)
  if (millis() - lastUIUpdate >= 1000) {
    lastUIUpdate = millis();

    // --- 1. SENSORS (Background) ---
    DHT.read(DHTPIN);
    float currentTemp = (float)DHT.temperature;
    float currentHum = (float)DHT.humidity;

    // --- 2. GLOBAL UI (Taskbar - Always Visible) ---
    displayBox.updateClock(tft, timeBox.getFormattedTime(), timeBox.getFormattedDate());
    displayBox.drawWifiIcon(tft, netBox.getSignalLevel());

    // --- 3. PAGE-SPECIFIC UI ---
    if (currentPage == WEATHER_PAGE) {
      displayBox.updateWeather(tft, currentTemp, currentHum);

      if (displayBox.isClockExpanded) {
        displayBox.drawExpandedClock(tft, timeBox.getFormattedTime(), timeBox.getFormattedDate(),
                                     timeBox.getDayName(), WiFi.SSID());
      }
    } else if (currentPage == SYSTEM_PAGE) {
      // You can call a small update function here for live system stats if needed
      // displayBox.updateSystemStats(tft);
    }
  }

  // --- 4. FIREBASE SYNC (Every 20 seconds) ---
  if (millis() - lastFirebaseSync >= 20000) {
    lastFirebaseSync = millis();
    netBox.updateFirebase((float)DHT.temperature, (float)DHT.humidity,
                          timeBox.getFormattedTime(), timeBox.getFormattedDate(), timeBox.getTimestamp());
  }

  // --- 5. TOUCH CONTROLS ---
  handleInput();
}

void handleInput() {
  uint16_t x, y;
  if (touchBox.isPressed(tft, x, y)) {
    x = 320 - x;  // Adjust for screen rotation

    // Start Button Logic
    if (x < 50 && y > 205) {
      if (!displayBox.isMenuOpen) {
        displayBox.drawStartMenu(tft);
      } else {
        displayBox.hideStartMenu(tft);
        repairPage();
      }
      delay(250);
      return;
    }

    // Taskbar Clock Logic
    if (x > 240 && y > 205) {
      if (!displayBox.isClockExpanded) {
        displayBox.drawExpandedClock(tft, timeBox.getFormattedTime(), timeBox.getFormattedDate(),
                                     timeBox.getDayName(), WiFi.SSID(), true);
      } else {
        displayBox.hideExpandedClock(tft);
        repairPage();
      }
      delay(250);
    }

    // Inside Start Menu Actions
    if (displayBox.isMenuOpen) {

      //WEATHER PAGE BUTTON (Check coordinates based on your menu layout)
      if (x > 5 && x < 100 && y > 130 && y < 152) {
        displayBox.hideStartMenu(tft);
        switchPage(WEATHER_PAGE);
      }

      // SYSTEM PAGE BUTTON
      if (x > 5 && x < 100 && y > 155 && y < 177) {
        displayBox.hideStartMenu(tft);
        switchPage(SYSTEM_PAGE);
      }

      // RESTART BUTTON
      if (x > 4 && x < 100 && y > 180 && y < 202) {
        tft.fillRect(0, 0, 320, 240, 0x0063);
        tft.unloadFont();
        tft.loadFont(ATR28);
        tft.setTextDatum(MC_DATUM);
        tft.setTextColor(TFT_WHITE);
        String text = String(SYS_REBOOTING);
        int newLinePos = text.indexOf('\n');
        if (newLinePos != -1) {
          tft.drawString(text.substring(0, newLinePos), 160, 106);
          tft.drawString(text.substring(newLinePos + 1), 160, 136);
        } else {
          tft.drawString(text, 160, 120);
        }
        SD.end();
        WiFi.disconnect(true);
        WiFi.mode(WIFI_OFF);
        delay(500);
        ESP.restart();
      }
    }
  }
}

/* * Force redraws the current UI components to repair the screen 
 * after a menu or overlay is closed.
 */
void repairPage() {
  // Clear only the workspace (between Header and Taskbar)
  tft.fillRect(0, 30, 320, 175, TFT_BLACK);

  if (currentPage == WEATHER_PAGE) {
    displayBox.drawWeatherPage(tft);
    // Immediately fill with current sensor data
    displayBox.updateWeather(tft, (float)DHT.temperature, (float)DHT.humidity);
  } else if (currentPage == SYSTEM_PAGE) {
    displayBox.drawSystemPage(tft);
  }
}

void switchPage(Page targetPage) {
  if (currentPage == targetPage) return;

  currentPage = targetPage;

  /* Clear only the workspace (Above the taskbar) 
       This prevents taskbar flickering */
  tft.fillRect(0, 0, 320, 205, TFT_BLACK);

  switch (currentPage) {
    case WEATHER_PAGE:
      displayBox.isClockExpanded = false;
      displayBox.isMenuOpen = false;

      displayBox.drawHeader(tft, WEATHER_TITLE, 0x0063, 0xFFFF);
      displayBox.drawWeatherPage(tft);
      break;

    case SYSTEM_PAGE:
      displayBox.isClockExpanded = false;
      displayBox.isMenuOpen = false;

      displayBox.drawHeader(tft, SYSTEM_TITLE, 0x0063, 0xFFFF);
      displayBox.drawSystemPage(tft);
      break;
    case NONE:
      // This case usually stays empty as it's just a transition state
      break;
  }
}