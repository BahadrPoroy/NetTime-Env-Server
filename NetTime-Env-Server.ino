#include <TFT_eSPI.h>
#include <dht11.h>
#include <SD.h>
#include "config.h"
#include "structs.h"
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

Page currentPage = NONE;

float currentTemp;
float currentHum;

// Global Variables of Feeder Client
bool isFed = false;
String feederStatus = "IDLE";
long lastFedTime = 0;
String lastDayChecked;
bool isFeederAlarmActive = true;
uint16_t feederAlarmColor;

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
  netBox.begin(tft, displayBox);
  timeBox.begin();
  netBox.readFirebase(isFed, lastFedTime);
  yield();

  tft.fillScreen(TFT_BLACK);
  displayBox.drawTaskbar(tft);
  switchPage(DESKTOP_PAGE);

  feederAlarmColor = isFed ? TFT_YELLOW : TFT_RED;
}

void loop() {
  netBox.handleOTA();
  netBox.handleFeederNetwork(feederStatus, isFed, lastFedTime, timeBox.getTimestamp());
  netBox.handleOpenWeather();

  // Unified UI and Sensor Update (Every 1 second)
  if (millis() - lastUIUpdate >= 1000) {
    lastUIUpdate = millis();

    // --- 1. SENSORS (Background) ---
    DHT.read(DHTPIN);
    currentTemp = (float)DHT.temperature;
    currentHum = (float)DHT.humidity;

    // --- 2. GLOBAL UI (Taskbar - Always Visible) ---
    displayBox.updateClock(tft, timeBox.getFormattedTime(), timeBox.getFormattedDate());
    displayBox.drawWifiIcon(tft, netBox.getSignalLevel());

    switch (currentPage) {
      case HOME_PAGE:
        displayBox.updateHome(tft, isFed, timeBox.getFormattedTime(), currentTemp);
        break;

      case WEATHER_PAGE:
        if (!displayBox.isClockExpanded && !displayBox.isMenuOpen) {
          displayBox.updateWeather(tft, currentPage, currentTemp, currentHum, netBox.currentWeather);
        }
        break;

      case SYSTEM_PAGE:
        if (!displayBox.isClockExpanded) {
          displayBox.updateSystemStats(tft, ESP.getFreeHeap());
        }
        break;

      case FEEDER_PAGE:
        displayBox.updateFeeder(tft, feederStatus, isFed);
        break;

      case SETTINGS_PAGE:
        break;

      default:
        break;
    }

    // 3. EXPANDED CLOCK (Overlay)
    if (displayBox.isClockExpanded) {
      displayBox.drawExpandedClock(tft, timeBox.getFormattedTime(), timeBox.getFormattedDate(),
                                   timeBox.getDayName(), WiFi.SSID());
    }
  }

  // --- Feeding Control ---
  if (timeBox.getHour() >= 12 && timeBox.getHour() <= 15) {
    if (feederStatus == "ERROR_HARDWARE") {
      feederAlarmColor = TFT_RED;  // Critical error, don't send command
    } else if (!isFed) {
      if (feederStatus == "PENDING") {
        feederAlarmColor = TFT_YELLOW;
      } else {
        if (timeBox.getTimestamp() - lastFedTime > 180) {
          netBox.broadcastUDP("FEED_NOW");
          feederAlarmColor = TFT_YELLOW;
        } else {
          feederAlarmColor = TFT_GREEN;  // If is fed on current day it's green
        }
      }
    } else if (timeBox.getHour() > 15) {
      feederAlarmColor = isFed ? TFT_GREEN : TFT_RED;
    } else if (timeBox.getHour() == 0 && timeBox.getMinute() == 0 && lastDayChecked != timeBox.getDayName()) {
      isFed = false;
      feederAlarmColor = TFT_YELLOW;
      lastDayChecked = timeBox.getDayName();
      feederStatus = "IDLE";
    }
  }

  // --- 4. FIREBASE SYNC (Every 20 seconds) ---
  if (millis() - lastFirebaseSync >= 20000) {
    lastFirebaseSync = millis();
    netBox.updateFirebase((float)DHT.temperature, (float)DHT.humidity,
                          timeBox.getFormattedTime(), timeBox.getFormattedDate(), timeBox.getTimestamp(), isFed, lastFedTime);
  }

  // --- 5. TOUCH CONTROLS ---
  handleInput();
  displayBox.drawSystemTray(tft, isFeederAlarmActive, feederAlarmColor);
}

void handleInput() {
  uint16_t x, y;
  if (!touchBox.isPressed(tft, x, y)) return;

  x = 320 - x;  // Adjust for screen rotation

  // --- Start Button Logic ---
  if (x < 50 && y > 205) {
    if (!displayBox.isMenuOpen) {
      if (displayBox.isClockExpanded)
        displayBox.hideExpandedClock(tft, currentPage, currentTemp, currentHum);
      displayBox.drawStartMenu(tft);
    } else {
      displayBox.hideStartMenu(tft, currentPage, currentTemp, currentHum);
    }
  }

  // --- Taskbar Clock Logic ---
  else if (x > 240 && y > 205) {
    if (!displayBox.isClockExpanded) {
      if (displayBox.isMenuOpen)
        displayBox.hideStartMenu(tft, currentPage, currentTemp, currentHum);
      displayBox.drawExpandedClock(tft, timeBox.getFormattedTime(), timeBox.getFormattedDate(),
                                   timeBox.getDayName(), WiFi.SSID(), true);
    } else {
      displayBox.hideExpandedClock(tft, currentPage, currentTemp, currentHum);
    }
  }

  // --- Start Menu Actions ---
  else if (displayBox.isMenuOpen) {
    if (x > 5 && x < 100 && y > 155 && y < 177) {
      switchPage(SYSTEM_PAGE);
    } else if (x > 4 && x < 100 && y > 180 && y < 202) {
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
      ESP.restart();
    }
  }
  // --- Page Specific Logic ---
  else if (currentPage == DESKTOP_PAGE) {
    if (y > 30 && y < 120) {
      if (x > 10 && x < 75) switchPage(SYSTEM_PAGE);
      else if (x > 85 && x < 155) switchPage(WEATHER_PAGE);
      else if (x > 165 && x < 235) switchPage(FEEDER_PAGE);
    }
  } else if (currentPage == FEEDER_PAGE) {
    if (x > 95 && x < 225 && y > 95 && y < 145 && !isFed) {
      netBox.broadcastUDP("FEED_NOW");
    }
  }
  if (currentPage != DESKTOP_PAGE) {
    if (x > 290 && y < 30) {
      switchPage(DESKTOP_PAGE);
    }
  }
  delay(250);
}


/* * Force redraws the current UI components to repair the screen 
 * after a menu or overlay is closed.
 */

void switchPage(Page targetPage) {
  if (currentPage == targetPage) return;
  currentPage = targetPage;
  displayBox.isMenuOpen = false;
  displayBox.isClockExpanded = false;
  /* Clear only the workspace (Above the taskbar)
  This prevents taskbar flickering */
  tft.fillRect(0, 0, 320, 205, TFT_BLACK);

  switch (currentPage) {
    case DESKTOP_PAGE:
      displayBox.drawDesktopPage(tft);
      break;
    case WEATHER_PAGE:
      displayBox.lastIcon = "";
      displayBox.drawHeader(tft, WEATHER_TITLE, 0x0063, 0xFFFF);
      displayBox.drawWeatherPage(tft);
      break;

    case SYSTEM_PAGE:
      displayBox.drawHeader(tft, SYSTEM_TITLE, 0x0063, 0xFFFF);
      displayBox.drawSystemPage(tft);
      break;

    case HOME_PAGE:
      displayBox.drawHeader(tft, HOME_TITLE, 0x0063, 0xFFFF);
      displayBox.drawHomePage(tft);
      break;

    case FEEDER_PAGE:
      displayBox.drawHeader(tft, FEEDER_TITLE, 0x0063, 0xFFFF);
      displayBox.drawFeederPage(tft);
      break;

    case SETTINGS_PAGE:
      break;

    default:
      break;
  }
}