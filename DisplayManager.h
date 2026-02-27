#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include "structs.h"
#include <TFT_eSPI.h>
#include <SD.h>
#include <LittleFS.h>
#include <ESP8266WiFi.h>  // Required to use the WiFi object inside this class
#include "language.h"
#include "Fonts/myFonts.h"

#define VERSION "NetTime OS v2.7.1 (2026)"

// Definition of icon height & width values
#define ICON_W 32
#define ICON_H 32
#define TRAY_ICON_H 16
#define TRAY_ICON_W 16

// --- System Tray Configuration ---
#define MAX_TRAY_ICONS 3  // Maximum number of visible icons before grouping
#define TRAY_ICON_W 16    // Width of each tray icon
#define TRAY_ICON_H 16    // Height of each tray icon
#define TRAY_SPACING 4    // Gap between icons
#define TRAY_Y_START 217  // Y position in the 210-240 Taskbar area

// ---- Feeder Alert variables ---
uint16_t lastFishColor = 0;  // To track state changes
bool lastFishState = false;

// --- RAM Bar Color Zones & RAM Values---
const uint32_t maxUsableHeap = 43008;  //42 KB Max Usable Ram Capacity
const uint32_t RAM_DANGER = 12000;     // 12 KB below is critical (red)
const uint32_t RAM_WARNING = 22000;    // 22 KB below is dangerous (yellow)

class DisplayManager {
private:
  unsigned long lastAnimMillis = 0;
  String lastDayName = "";

public:
  /**
     * @brief Loads and displays a 16-bit BMP image from the SD card.
     * @param tft Reference to the TFT object.
     * @param filename Path to the BMP file (e.g., "logo.bmp").
     * @param x X coordinate for the top-left corner.
     * @param y Y coordinate for the top-left corner.
     */
  String lastIcon = "";

  void init(TFT_eSPI &tft) {
    tft.init();
    tft.setRotation(3);
    tft.fillScreen(TFT_BLACK);
  }

  void drawSDImage(TFT_eSPI &tft, const char *filename, int x, int y, int w, int h) {
    File imgFile = SD.open(filename, FILE_READ);
    if (!imgFile) return;

    uint32_t dataOffset;
    imgFile.seek(10);
    imgFile.read((uint8_t *)&dataOffset, 4);

    imgFile.seek(dataOffset);

    for (int row = h - 1; row >= 0; row--) {
      for (int col = 0; col < w; col++) {
        uint16_t color;
        imgFile.read((uint8_t *)&color, 2);
        tft.drawPixel(x + col, y + row, color);
      }
    }
    // ---------------------------------
    imgFile.close();
  }

  void drawMonoIcon(TFT_eSPI &tft, const char *filename, int x, int y, int w, int h, uint16_t color) {  //Scalebility added
    File imgFile = SD.open(filename, FILE_READ);
    if (!imgFile) return;

    imgFile.seek(62);

    // Calculate bytes per row: BMP rows are padded to 4-byte boundaries
    int bytesPerRow = (w + 31) / 32 * 4;

    for (int i = 0; i < h; i++) {  // Y axis
      uint8_t rowData[bytesPerRow];
      imgFile.read(rowData, bytesPerRow);

      for (int j = 0; j < w; j++) {  // X axis as byte
                                     // Calculate which byte and which bit we are looking at
        int byteIdx = j / 8;
        int bitIdx = j % 8;

        // Check if the specific bit is active (1)
        if (rowData[byteIdx] & (0x80 >> bitIdx)) {
          // BMP stores rows bottom-to-top, so we use (h - 1 - i)
          tft.drawPixel(x + j, y + (h - 1 - i), color);
        }
      }
    }
    imgFile.close();
  }

  void drawWeatherIcon(TFT_eSPI &tft, String iconName, int x, int y, int w, int h) {
    // File Path Creation (Depends on folder struct "/Weather_icons/" changable)
    String fileName = "/Weather_Icons/" + iconName + ".bmp";

    File imgFile = SD.open(fileName, FILE_READ);
    if (!imgFile) {
      Serial.print("Icon error: ");
      Serial.println(fileName);
      return;
    }
    imgFile.seek(54);

    int bytesPerRow = ((w * 16 + 31) / 32) * 4;

    for (int i = 0; i < h; i++) {

      uint8_t rowData[bytesPerRow];
      imgFile.read(rowData, bytesPerRow);

      for (int j = 0; j < w; j++) {
        uint16_t b = rowData[j * 2];
        uint16_t a = rowData[j * 2 + 1];
        uint16_t color = (a << 8) | b;
        if (color == 0xFFFF) color = 0x0000;
        tft.drawPixel(x + j, y + (h - 1 - i), color);
      }
    }
    imgFile.close();
  }


  /**
     * @brief Shows the "POROY SOFTWARE" splash screen with a loading animation.
     */
  void drawStaticSplash(TFT_eSPI &tft) {
    tft.fillScreen(TFT_BLACK);
    // Load the logo immediately
    drawSDImage(tft, "/logo.bmp", 0, 0, 320, 240);
  }

  int scanX = 85;
  bool direction = true;

  void updateLoadingAnimation(TFT_eSPI &tft) {
    // Run animation frame every 40ms without using delay()
    if (millis() - lastAnimMillis >= 40) {
      // Erase previous position
      int startPoint = 85;
      int endPoint = 235;

      tft.drawFastHLine(scanX, 220, 20, 0x0000);

      if (direction) scanX += 2;
      else scanX -= 2;
      if (scanX > endPoint - 20 || scanX < startPoint) direction = !direction;

      tft.drawFastHLine(scanX, 200, 20, TFT_CYAN);
      lastAnimMillis = millis();
    }
  }

  bool isMenuOpen = false;
  bool isClockExpanded = false;

  void drawStartMenu(TFT_eSPI &tft) {
    // 1. Menu Container (Extended height to fit 2 items)
    // Positioned from Y: 145 to 205
    tft.loadFont(ATR12);
    tft.setTextColor(TFT_WHITE, 0x02D7);
    tft.fillRect(2, 145, 100, 60, 0x10A2);
    tft.drawRect(2, 145, 100, 60, 0x319F);

    tft.setTextDatum(MC_DATUM);

    // 2. System Page Button (Middle)
    tft.fillRect(5, 155, 94, 22, 0x02D7);
    tft.drawString(SYSTEM_BTN, 52, 166);

    // 3. Restart Button (Bottom)
    // Red or distinct blue to indicate action

    tft.fillRect(5, 180, 94, 22, 0x02D7);
    String text = String(TXT_RESTART);
    int newLinePos = text.indexOf('\n');
    if (newLinePos != -1) {
      tft.drawString(text.substring(0, newLinePos), 52, 186);
      tft.drawString(text.substring(newLinePos + 1), 52, 195);
    } else {
      tft.drawString(text, 52, 191);
    }

    // 5. State Management
    isMenuOpen = true;
    tft.unloadFont();
    tft.setTextPadding(0);
    tft.setTextDatum(TL_DATUM);
  }

  //-------------- HOME_PAGE -------------------

  void drawHomePage(TFT_eSPI &tft) {
    int startX = 10;
    int startY = 40;
    int spaceX = 30;
    int spaceY = 25;
    int centerX = 160;
    tft.loadFont(ATR16);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextDatum(TC_DATUM);

    // seperators
    tft.drawFastHLine(0, startY + (2 * spaceY), 320, 0x319F);
    tft.drawFastHLine(0, startY + (3.5 * spaceY) + 5, 320, 0x319F);

    // labels
    tft.drawString(String(TXT_TEMP), centerX, startY);
    tft.setTextDatum(TL_DATUM);
    tft.unloadFont();

    tft.loadFont(ATR20);
    tft.drawString(String(TXT_INDOOR_TEMP) + ":", startX, startY + spaceY);
    int secondRow = startX + tft.textWidth(TXT_INDOOR_TEMP) + tft.textWidth(": 88 °C") + spaceX;
    if (secondRow <= 160) secondRow = centerX + spaceX;
    tft.drawString(String(TXT_OUTDOOR_TEMP) + ":", secondRow, startY + spaceY);
    tft.unloadFont();

    tft.loadFont(ATR16);
    tft.setTextDatum(TC_DATUM);
    tft.drawString(String(TXT_FEEDER), centerX, startY + (4 * spaceY));

    tft.unloadFont();
  }

  //-------------- HOME_PAGE END ------------------

  //-------------- UPDATE_HOME_PAGE ---------------

  void updateHome(TFT_eSPI &tft, const String &feederStatus, bool isFed, String timeStr, float intemp, const WeatherData &weather) {

    int startX;
    int startY = 40;
    int spaceX = 30;
    int spaceY = 25;
    int centerX = 160;
    tft.setTextDatum(TL_DATUM);

    // Temperature
    tft.loadFont(ATR20);
    tft.setTextColor(0x04DF, TFT_BLACK);
    tft.setTextPadding(tft.textWidth("88.8 °C"));
    tft.drawString(String(intemp, 1) + " °C", tft.textWidth(TXT_INDOOR_TEMP) + spaceX, startY + spaceY);
    int secondRow = startX + tft.textWidth(TXT_INDOOR_TEMP) + tft.textWidth(": 88 °C") + spaceX;
    if (secondRow <= 160) secondRow = centerX + tft.textWidth(TXT_INDOOR_TEMP) + 2 * spaceX;
    tft.drawString(String(weather.temp, 1) + " °C", secondRow, startY + spaceY);
    tft.unloadFont();

    // Clock
    tft.loadFont(ATR24);
    tft.setTextDatum(TC_DATUM);
    tft.setTextColor(0x04DF, TFT_BLACK);
    tft.setTextPadding(tft.textWidth("88:88:88"));
    tft.drawString(timeStr, centerX, startY + 2.5 * spaceY);
    tft.unloadFont();

    // IsFed?
    tft.loadFont(ATR20);
    uint16_t statusColor;
    String statusText;
    if (feederStatus == "PENDING" || feederStatus == "IDLE" && !isFed) {
      statusColor = 0xFCA0;
      statusText = String(TXT_WAIT);
    } else if (feederStatus == "SUCCESS" || isFed) {
      statusColor = TFT_GREEN;
      statusText = String(TXT_FED);
    } else {
      statusColor = TFT_RED;
      statusText = String(TXT_ERR);
    }
    tft.setTextColor(statusColor, TFT_BLACK);
    tft.setTextPadding(tft.textWidth("Yemleme Bekliyor"));
    tft.drawString(statusText, centerX, startY + 5 * spaceY);
    tft.unloadFont();

    tft.setTextPadding(0);
  }

  //------------ UPDATE_HOME_PAGE END -------------

  //----- FEEDER_PAGE -----
  void drawFeederPage(TFT_eSPI &tft) {
    tft.loadFont(ATR20);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextDatum(TL_DATUM);

    // Etiketler
    tft.drawString(String(TXT_FEEDER) + ":", 10, 35);
    tft.fillRect(100, 100, 120, 40, 0x04DF);
    tft.setTextDatum(MC_DATUM);
    tft.setTextColor(TFT_WHITE, 0x04DF);
    tft.setTextPadding(tft.textWidth(FEEDER_BTN));
    tft.drawString(FEEDER_BTN, 160, 120);
    tft.unloadFont();
    tft.setTextPadding(0);
  }
  //----- FEEDER_PAGE END -----


  //-------------- UPDATE_FEEDER_PAGE ---------------

  void updateFeeder(TFT_eSPI &tft, const String &feederStatus, const bool &isFed) {
    tft.loadFont(ATR20);
    tft.setTextDatum(TL_DATUM);
    uint16_t statusColor;
    String statusText;
    if (feederStatus == "PENDING" || feederStatus == "IDLE" && !isFed) {
      statusColor = 0xFCA0;
      statusText = String(TXT_WAIT);
    } else if (feederStatus == "SUCCESS" || isFed) {
      statusColor = TFT_GREEN;
      statusText = String(TXT_FED);
    } else {
      statusColor = TFT_RED;
      statusText = String(TXT_ERR);
    }
    tft.setTextColor(statusColor, TFT_BLACK);
    tft.setTextPadding(tft.textWidth("Yemleme Bekliyor"));
    tft.drawString(statusText, 110, 35);

    tft.unloadFont();
    tft.setTextPadding(0);
  }

  void hideStartMenu(TFT_eSPI &tft, Page currentPage, float temp, float hum) {
    isMenuOpen = false;
    tft.setViewport(2, 145, 100, 60, false);
    repairPage(tft, currentPage, temp, hum);
    tft.resetViewport();
  }

  void drawTaskbar(TFT_eSPI &tft) {
    // 1. Higher Taskbar area (from y:205 to 240)
    tft.fillRect(0, 205, 320, 35, 0x0112);
    tft.drawFastHLine(0, 205, 320, 0x319F);  // Taskbar border

    // 2. Resized Start Button (Centered in the new bar)
    drawSDImage(tft, "/start.bmp", 5, 206, 32, 32);
  }

  void drawHeader(TFT_eSPI &tft, const char *title, uint16_t bgColor, uint16_t txtColor) {
    tft.fillRect(0, 0, 320, 30, bgColor);
    // Close Button (X) Configuration
    int btnSize = 30;          // Button height matches header height
    int btnX = 320 - btnSize;  // Flush to the right edge
    int btnY = 0;
    // Draw the red background for the close button
    tft.fillRect(btnX, btnY, btnSize, btnSize, TFT_RED);
    // Draw the "X" symbol using diagonal lines
    int padding = 9;  // Padding inside the button for the X icon
    uint16_t xColor = TFT_WHITE;
    // Line 1: Top-Left to Bottom-Right
    tft.drawLine(btnX + padding, btnY + padding, btnX + btnSize - padding, btnY + btnSize - padding, xColor);
    // Line 2: Top-Right to Bottom-Left
    tft.drawLine(btnX + btnSize - padding, btnY + padding, btnX + padding, btnY + btnSize - padding, xColor);

    tft.loadFont(ATR24);
    tft.setTextDatum(MC_DATUM);
    tft.setTextColor(txtColor, bgColor);
    // Optical Center: Calculate the middle point between the left edge and the button start
    int opticalCenter = btnX / 2;
    tft.drawString(title, opticalCenter, 15);
    tft.unloadFont();
  }

  //---------- WEATHER_PAGE ----------

  void drawWeatherPage(TFT_eSPI &tft) {
    //Indoor Section Title
    tft.loadFont(ATR16);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);  // White for labels
    tft.setTextDatum(MC_DATUM);
    tft.drawString(TXT_INDOOR, 160, 50);  // Label from language.h
    tft.unloadFont();

    //Indoor Section labels
    tft.loadFont(ATR20);
    tft.setTextDatum(TL_DATUM);
    // Draw the static labels once
    tft.drawString(String(TXT_TEMP) + ":", 10, 65);
    tft.drawString(String(TXT_HUM) + ":", 200, 65);
    tft.unloadFont();

    // Visual divider using the taskbar border color
    tft.drawFastHLine(0, 90, 320, 0x319F);

    // Outdoor Section Title
    tft.loadFont(ATR16);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);  // White for labels
    tft.setTextDatum(MC_DATUM);
    tft.drawString(TXT_OUTDOOR, 160, 120);  // Label from language.h
    tft.unloadFont();

    // Outdoor Section labels
    tft.loadFont(ATR20);
    tft.setTextDatum(TL_DATUM);
    // Draw the static labels once
    tft.drawString(String(TXT_TEMP) + ":", 130, 140);
    tft.drawString(String(TXT_HUM) + ":", 130, 170);
    tft.unloadFont();
    tft.setTextPadding(0);
  }

  //---------- WEATHER_PAGE END ----------

  // --- Dynamic Data Only ---
  void updateWeather(TFT_eSPI &tft, Page currentPage, float roomTemp, float roomHum, const WeatherData &weather) {

    // 1. Update Indoor Data (DHT11)
    tft.loadFont(ATR20);
    tft.setTextColor(0x04DF, TFT_BLACK);  // Cyan for values
    tft.setTextDatum(TL_DATUM);

    // Update Temperature Value
    tft.setTextPadding(tft.textWidth("88.8 °C"));
    tft.drawString(String(roomTemp, 1) + " °C", 90, 65);

    // Update Humidity Value
    tft.setTextPadding(tft.textWidth("%100"));
    tft.drawString("%" + String(roomHum, 0), 260, 65);

    // 2. Update Outdoor Data (OpenWeather)
    if (weather.updated) {
      // Weather Icon
      if (currentPage == WEATHER_PAGE && weather.icon != lastIcon) {
        tft.fillRect(19, 129, 66, 66, TFT_BLACK);
        drawWeatherIcon(tft, weather.icon, 20, 130, 64, 64);
        lastIcon = weather.icon;
        tft.fillRect(18, 192, 5, 5, TFT_BLACK);
      }
      // 2. Outdoor values
      tft.setTextDatum(TL_DATUM);
      tft.setTextPadding(tft.textWidth("88.8 °C"));
      tft.drawString(String(weather.temp, 1) + " °C", 210, 140);

      tft.setTextPadding(tft.textWidth("%100"));
      tft.drawString("%" + String(weather.humidity), 210, 170);
    }
    tft.unloadFont();
    tft.setTextPadding(0);
  }

  void updateClock(TFT_eSPI &tft, String timeStr, String dateStr) {
    tft.unloadFont();

    tft.loadFont(ATR16);
    tft.setTextDatum(TR_DATUM);  // Align to Right
    tft.setTextColor(TFT_WHITE, 0x0112);

    // Time on Taskbar (Top row of tray)
    // Show only HH:MM for tray, keep it clean
    tft.setTextPadding(tft.textWidth("88:88"));
    tft.drawString(timeStr.substring(0, 5), 315, 208);

    // Date on Taskbar (Bottom row of tray)
    tft.loadFont(ATR12);
    tft.setTextDatum(TR_DATUM);
    tft.setTextPadding(tft.textWidth("88/88/8888"));
    tft.drawString(dateStr, 315, 226);

    tft.setTextDatum(TL_DATUM);  // Reset
    tft.unloadFont();
    tft.setTextPadding(0);
  }

  void drawExpandedClock(TFT_eSPI &tft, String fullTime, String date, String day, String ssid, bool firstDraw = false) {
    // Semi-transparent look window
    if (firstDraw) {
      tft.fillRect(140, 65, 175, 140, 0x0112);
      tft.drawRect(140, 65, 175, 140, 0x319F);
      lastDayName = "";  // Reset tracker on first draw
    }
    tft.loadFont(ATR24);
    tft.setTextDatum(MC_DATUM);

    // Full Time with Seconds (Large)
    tft.setTextPadding(tft.textWidth("88:88:88"));
    tft.setTextColor(TFT_WHITE, 0x0112);
    tft.drawString(fullTime, 227, 90);

    tft.setTextColor(TFT_WHITE, 0x0112);
    tft.setTextPadding(tft.textWidth("88/88/8888"));
    tft.drawString(date, 227, 125);
    tft.unloadFont();

    // Day Name (From language.h)
    tft.loadFont(ATR20);
    tft.setTextDatum(MC_DATUM);
    tft.setTextColor(TFT_WHITE, 0x0112);

    // Trigger cleanup only when the day actually changes
    if (day != lastDayName) {
      // Eski ve yeni günün piksel genişliklerini al
      int oldW = tft.textWidth(lastDayName) + 10;  // 10px safety space
      int clearH = tft.fontHeight();               // Height of loaded font
      tft.fillRect(227 - (oldW / 2), 155 - (clearH / 2), oldW, clearH, 0x0112);

      tft.setTextPadding(tft.textWidth(day));
      tft.drawString(day, 227, 155);

      lastDayName = day;  // Update tracker
    } else {
      // Standard draw with current width padding for stability
      tft.setTextPadding(tft.textWidth(day));
      tft.drawString(day, 227, 155);
    }

    tft.unloadFont();

    tft.loadFont(ATR16);
    tft.setTextDatum(MC_DATUM);
    tft.setTextColor(TFT_WHITE, 0x0112);
    tft.setTextPadding(160);
    tft.drawString(ssid, 227, 185);
    tft.setTextPadding(0);
    tft.unloadFont();
    isClockExpanded = true;
  }

  void hideExpandedClock(TFT_eSPI &tft, Page currentPage, float temp, float hum) {
    isClockExpanded = false;
    tft.setViewport(140, 65, 175, 140, false);
    repairPage(tft, currentPage, temp, hum);
    tft.resetViewport();
  }

  void showUpdateScreen(TFT_eSPI &tft, int progress) {

    tft.loadFont(ATR16);
    tft.setTextDatum(MC_DATUM);

    if (progress == 0) {
      tft.fillScreen(TFT_BLACK);
      tft.setTextColor(TFT_CYAN, TFT_BLACK);
      tft.drawString(String(SYS_UPDATE), 160, 80);

      tft.drawRect(60, 110, 200, 20, TFT_WHITE);

      tft.setTextColor(TFT_ORANGE, TFT_BLACK);
      tft.drawString(String(TXT_UPDATE_NO_POWER), 160, 170);
    }

    tft.fillRect(62, 112, (progress * 196) / 100, 16, 0x0112);

    tft.setTextColor(TFT_CYAN, TFT_BLACK);

    tft.setTextPadding(tft.textWidth("100%"));
    tft.drawString(String(progress) + "%", 160, 145);

    tft.setTextPadding(0);
    tft.unloadFont();
  }
  void drawWifiIcon(TFT_eSPI &tft, int signalLevel) {
    // Positioning
    const int x_base = 225;
    const int y_bottom = 228;
    const int max_h = 12;  // Maximum bar height
    const int bar_w = 3;
    const int bar_gap = 2;

    // Color Definitions
    uint16_t activeColor = 0xFFFF;    // White for V2 look
    uint16_t inactiveColor = 0x10A4;  // Deep Charcoal (looks dimmed)
    uint16_t bgColor = 0x0112;        // Your specific taskbar blue

    for (int i = 1; i <= 4; i++) {
      int x_pos = x_base + (i * (bar_w + bar_gap));
      int current_bar_h = i * 3;

      // 1. CLEAR: Wipe the entire bar column area first to prevent ghosting
      // We wipe the max possible height (4 * 3 = 12px)
      tft.fillRect(x_pos, y_bottom - max_h, bar_w, max_h, bgColor);

      // 2. DRAW: Draw the bar with the appropriate state color
      uint16_t color = (i <= signalLevel) ? activeColor : inactiveColor;
      tft.fillRect(x_pos, y_bottom - current_bar_h, bar_w, current_bar_h, color);
    }
  }

  //------ DESKTOP PAGE -----
  void drawDesktopPage(TFT_eSPI &tft) {
    tft.fillRect(0, 0, 320, 30, TFT_BLACK);
    tft.loadFont(ATR16);
    tft.setTextDatum(BC_DATUM);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    // Grid Settings
    int startX = 36;
    int startY = 40;
    int spacingX = 82;
    int spacingY = 90;
    int nameSpacing = 52;
    //---------------

    //Home
    drawMonoIcon(tft, "/Page_Icons/home.bmp", startX - 16, startY, ICON_W, ICON_H, 0x04DF);
    tft.setTextPadding(tft.textWidth(HOME_BTN));
    tft.drawString(HOME_BTN, startX, startY + nameSpacing);

    // System
    drawMonoIcon(tft, "/Page_Icons/system.bmp", (startX + spacingX) - 16, startY, ICON_W, ICON_H, 0x04DF);
    tft.setTextPadding(tft.textWidth(SYSTEM_BTN));
    tft.drawString(SYSTEM_BTN, startX + spacingX, startY + nameSpacing);

    // Weather
    drawMonoIcon(tft, "/Page_Icons/weather.bmp", (startX + 2 * spacingX) - 16, startY, ICON_W, ICON_H, 0x04DF);
    tft.setTextPadding(tft.textWidth(WEATHER_BTN));
    tft.drawString(WEATHER_BTN, (startX + 2 * spacingX), startY + nameSpacing);

    // Feeder
    drawMonoIcon(tft, "/Page_Icons/feeder.bmp", (startX + 3 * spacingX) - 16, startY, ICON_W, ICON_H, 0x04DF);
    tft.setTextPadding(tft.textWidth(FEEDER_BTN));
    tft.drawString(TXT_FEEDER, (startX + 3 * spacingX), startY + nameSpacing);

    // Settings
    drawMonoIcon(tft, "/Page_Icons/settings.bmp", startX - 16, startY + spacingY, ICON_W, ICON_H, 0x04DF);
    tft.setTextPadding(tft.textWidth(SETTINGS_BTN));
    tft.drawString(SETTINGS_BTN, startX, (startY + spacingY) + nameSpacing);
    tft.unloadFont();
    tft.setTextPadding(0);
  }
  //----- DESKTOP PAGE END -----

  /* * * Draws the System Properties page content.
   * Displays static device information.
   */
  void drawSystemPage(TFT_eSPI &tft) {
    // Draw the static system icon
    drawSDImage(tft, "/start.bmp", 15, 45, 32, 32);

    tft.loadFont(ATR16);
    tft.setTextDatum(TL_DATUM);

    int startY = 45;
    int lineSpacing = 19;

    // Header section
    tft.setTextColor(0x04DF, TFT_BLACK);
    tft.drawString("NetTime Pro " + String(TXT_CONSOLE), 80, startY);

    tft.unloadFont();
    tft.loadFont(ATR12);

    // Network Info (Static)
    tft.drawString(String(TXT_NET) + ": " + WiFi.SSID(), 80, startY + lineSpacing);
    tft.drawString("IP: " + WiFi.localIP().toString(), 80, startY + (lineSpacing * 2));

    // Visual separator
    tft.drawFastHLine(15, 120, 290, 0x319F);

    // Hardware Stats Section (Static labels)
    int statsY = 130;
    tft.drawString(String(SYS_RAM) + ": ", 40, statsY);
    tft.drawRect(180, statsY - 6, 100, 12, 0x04DF);  // RAM Bar Frame

    uint32_t flashSize = ESP.getFlashChipRealSize();
    tft.setTextDatum(TC_DATUM);
    tft.drawString(String(SYS_FLASH) + ": " + String(flashSize / 1048576) + " MB", 160, statsY + lineSpacing);

    tft.drawString(String(SYS_CPU) + ": " + String(ESP.getCpuFreqMHz()) + " MHz", 160, statsY + (lineSpacing * 2));

    // Footer Version Info
    tft.setTextDatum(BC_DATUM);
    tft.drawString(String(VERSION), 160, 200);

    tft.unloadFont();
    tft.setTextPadding(0);
    tft.setTextDatum(TL_DATUM);
  }

  /* * * Updates dynamic system data (RAM, CPU, Bar)
   * This is called inside the loop when SYSTEM_PAGE is active.
   */
  void updateSystemStats(TFT_eSPI &tft, uint32_t freeHeap) {
    tft.loadFont(ATR12);
    tft.setTextColor(0x04DF, TFT_BLACK);  // Using original Cyan color
    tft.setTextDatum(TL_DATUM);

    int statsY = 130;  // Must match drawSystemPage
    int ls = 19;       // Line spacing

    // Update RAM Numeric Value (Positioned next to label)
    tft.setTextPadding(tft.textWidth("88.8 KB"));
    tft.drawString(String(freeHeap / 1024.0, 1) + " KB", 105, statsY);

    int usedHeap = maxUsableHeap - freeHeap;
    if (usedHeap < 0) usedHeap = 0;

    // Update RAM Bar Fill (Mapped to 100px frame)
    int barMax = 96;
    int fillWidth = map(usedHeap, 0, maxUsableHeap, 0, barMax);

    if (fillWidth > barMax) fillWidth = barMax;
    if (fillWidth < 0) fillWidth = 0;

    uint16_t barColor;
    if (freeHeap < RAM_DANGER) barColor = TFT_RED;
    else if (freeHeap < RAM_WARNING) barColor = TFT_YELLOW;
    else barColor = TFT_GREEN;

    // Clear previous bar and draw new one
    tft.fillRect(182, statsY - 4, barMax, 8, TFT_BLACK);
    tft.fillRect(182, statsY - 4, fillWidth, 8, barColor);

    tft.unloadFont();
    tft.setTextPadding(0);
  }

  void repairPage(TFT_eSPI &tft, Page currentPage, float temp, float hum) {

    tft.fillRect(0, 0, 320, 240, TFT_BLACK);

    switch (currentPage) {
      case WEATHER_PAGE:
        lastIcon = "";
        drawWeatherPage(tft);
        break;

      case SYSTEM_PAGE:
        drawSystemPage(tft);
        break;

      case FEEDER_PAGE:
        drawFeederPage(tft);
        break;

      case HOME_PAGE:
        drawHomePage(tft);
        break;

      case SETTINGS_PAGE:
        break;

      case DESKTOP_PAGE:
        drawDesktopPage(tft);
        break;

      default:
        break;
    }
  }

  //----- SYSTEM TRAY -----
  /**
   * @brief Draws icons in the system tray area, shifting left from the WiFi icon.
   * @param feederAlarm Current state of the feeder alert.
   * @param feederColor Color for the feeder icon (White/Red).
   */
  void drawSystemTray(TFT_eSPI &tft, const bool &feederAlarm, uint16_t feederAlarmColor) {
    static uint16_t lastColor = 0;
    static int lastIconCount = -1;

    if (feederAlarmColor == lastColor) {
      return;
    }

    lastColor = feederAlarmColor;
    // Starting X (Just left of WiFi at 225)
    int cursorX = 220;
    int activeIconCounter = 0;
    if (feederAlarm) activeIconCounter++;

    if (activeIconCounter == 0) tft.fillRect(160, TRAY_Y_START, 65, 16, 0x0112);  // Cleans the area

    if (activeIconCounter <= MAX_TRAY_ICONS) {
      // SCENARIO 1: Everything fits
      if (feederAlarm) {
        cursorX -= TRAY_ICON_W;
        drawMonoIcon(tft, "/feeder_tray.bmp", cursorX, TRAY_Y_START, TRAY_ICON_W, TRAY_ICON_H, feederAlarmColor);
        cursorX -= TRAY_SPACING;
      }
    } else {
      // SCENARIO 2: Overflow (Windows Style)
      // Draw +X Indicator
      int hiddenCount = activeIconCounter - (MAX_TRAY_ICONS);
      tft.loadFont(ATR12);
      tft.setTextDatum(MR_DATUM);
      tft.setTextColor(TFT_WHITE, 0x0112);  // Taskbar blue
      tft.drawString("+" + String(hiddenCount), cursorX, TRAY_Y_START + 4);
      tft.unloadFont();

      cursorX -= 22;  // Space for "+2" etc.
      // Draw the remaining (MAX_TRAY_ICONS - 1) icons
      if (feederAlarm) {
        cursorX -= TRAY_ICON_W;
        drawMonoIcon(tft, "/feeder_tray.bmp", cursorX, TRAY_Y_START, TRAY_ICON_W, TRAY_ICON_H, feederAlarmColor);
        cursorX -= TRAY_SPACING;
      }
    }
  }
  //----- SYSTEM TRAY END -----
};
#endif