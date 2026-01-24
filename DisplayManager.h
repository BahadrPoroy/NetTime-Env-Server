#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include <TFT_eSPI.h>
#include <SD.h>
#include <LittleFS.h>
#include <ESP8266WiFi.h>  // Required to use the WiFi object inside this class
#include "language.h"
#include "myFonts.h"

class DisplayManager {
private:
  unsigned long lastAnimMillis = 0;

public:
  /**
     * @brief Loads and displays a 16-bit BMP image from the SD card.
     * @param tft Reference to the TFT object.
     * @param filename Path to the BMP file (e.g., "logo.bmp").
     * @param x X coordinate for the top-left corner.
     * @param y Y coordinate for the top-left corner.
     */
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
    // 1. Menu Container (Extended height to fit 3 items)
    // Positioned from Y: 125 to 205
    tft.fillRect(2, 125, 100, 80, 0x10A2);
    tft.drawRect(2, 125, 100, 80, 0x319F);

    tft.setTextDatum(MC_DATUM);

    // 2. Weather Page Button (Top)
    // Blue background for consistency
    tft.fillRect(5, 130, 94, 22, 0x02D7);
    tft.loadFont(ATR12);
    tft.setTextColor(TFT_WHITE, 0x02D7);
    tft.drawString(WEATHER_BTN, 52, 141);  // You can use a constant like WEATHER_BTN

    // 3. System Page Button (Middle)
    tft.fillRect(5, 155, 94, 22, 0x02D7);
    tft.drawString(SYSTEM_BTN, 52, 166);

    // 4. Restart Button (Bottom)
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

  void hideStartMenu(TFT_eSPI &tft) {
    // Clear menu area with black
    tft.fillRect(2, 120, 100, 85, TFT_BLACK);
    isMenuOpen = false;
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
    tft.loadFont(ATR24);
    tft.setTextDatum(MC_DATUM);
    tft.setTextColor(txtColor, bgColor);
    tft.drawString(title, 160, 15);
    tft.unloadFont();
  }

  void drawWeatherPage(TFT_eSPI &tft) {
    tft.loadFont(ATR20);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);  // White for labels
    tft.setTextDatum(TL_DATUM);

    // Draw the static labels once
    tft.drawString(String(TXT_TEMP) + ":", 10, 35);
    tft.drawString(String(TXT_HUM) + ":", 200, 35);

    tft.unloadFont();
  }

  // --- Dynamic Data Only ---
  void updateWeather(TFT_eSPI &tft, float temp, float hum) {
    tft.loadFont(ATR20);
    tft.setTextColor(0x04DF, TFT_BLACK);  // Cyan for values
    tft.setTextDatum(TL_DATUM);

    // Update Temperature Value
    // We start drawing after the label "Temp: " (roughly at X: 80-90 depending on font)
    tft.setTextPadding(tft.textWidth("88.8 °C"));
    tft.drawString(String(temp, 1) + " °C", 90, 35);

    // Update Humidity Value
    tft.setTextPadding(tft.textWidth("%100"));
    tft.drawString("%" + String(hum, 0), 260, 35);

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
    tft.drawString(timeStr.substring(0, 5), 315, 206);

    // Date on Taskbar (Bottom row of tray)
    tft.loadFont(ATR12);
    tft.setTextDatum(TR_DATUM);
    tft.setTextPadding(tft.textWidth("88/88/8888"));
    tft.drawString(dateStr, 315, 224);

    tft.setTextDatum(TL_DATUM);  // Reset
    tft.unloadFont();
    tft.setTextPadding(0);
  }

  void drawExpandedClock(TFT_eSPI &tft, String fullTime, String date, String day, String ssid, bool firstDraw = false) {
    // Semi-transparent look window
    if (firstDraw) {
      tft.fillRect(140, 65, 175, 140, 0x0112);
      tft.drawRect(140, 65, 175, 140, 0x319F);
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
    tft.setTextPadding(tft.textWidth("          "));
    tft.drawString(day, 227, 155);
    tft.unloadFont();

    tft.loadFont(ATR16);
    tft.setTextDatum(MC_DATUM);
    tft.setTextColor(TFT_WHITE, 0x0112);
    tft.setTextPadding(160);
    tft.drawString(ssid, 227, 185);
    tft.unloadFont();
    isClockExpanded = true;
  }

  void hideExpandedClock(TFT_eSPI &tft) {
    tft.fillRect(140, 65, 177, 140, TFT_BLACK);
    isClockExpanded = false;
  }

  // Add this inside DisplayManager class in DisplayManager.h

  void showUpdateScreen(TFT_eSPI &tft, int progress) {
    // Standard size fonts to ensure they fit 320px width
    if (progress == 0) {
      tft.fillScreen(TFT_BLACK);
      tft.loadFont(ATR16);
      tft.setTextColor(0x0112, TFT_BLACK);
      tft.setTextDatum(MC_DATUM);

      tft.drawString(String(SYS_UPDATE), 160, 80);
      tft.unloadFont();
      // Draw progress bar frame
      tft.drawRect(60, 110, 200, 20, TFT_WHITE);
    }

    // Fill progress bar
    tft.fillRect(62, 112, (progress * 196) / 100, 16, 0x0112);

    // Small percentage text below the bar
    tft.loadFont(ATR8);
    tft.setTextDatum(MC_DATUM);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);

    tft.setTextPadding(tft.textWidth("100% "));
    tft.drawString(String(progress) + "%", 160, 140);

    tft.setTextPadding(0);
    tft.setTextColor(TFT_ORANGE, TFT_BLACK);
    tft.drawString(String(TXT_UPDATE_NO_POWER), 160, 170);

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

  /* * Draws the System Properties page content.
   * Displays static device information using custom fonts.
   */
  void drawSystemPage(TFT_eSPI &tft) {
    // 1. Drawing the "Computer" or "System" Icon Placeholder
    drawSDImage(tft, "/start.bmp", 15, 45, 32, 32);

    tft.loadFont(ATR16);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextDatum(TL_DATUM);

    // 2. Hardware Section
    int startY = 45;
    int lineSpacing = 19;

    tft.setTextColor(0x04DF, TFT_BLACK);  // Cyan for the main title
    tft.drawString("NetTime Pro " + String(TXT_CONSOLE), 80, startY);

    tft.unloadFont();
    tft.loadFont(ATR12);

    // IP and Connection Info
    tft.drawString(String(TXT_NET) + ": " + WiFi.SSID(), 80, startY + lineSpacing);
    tft.drawString("IP: " + WiFi.localIP().toString(), 80, startY + (lineSpacing * 2));

    // 3. Visual Separator Line
    tft.drawFastHLine(15, 120, 290, 0x319F);

    // 4. Memory and Storage Info
    // Using KB and MB for a more technical look
    tft.drawString(String(SYS_RAM) + ": " + String(ESP.getFreeHeap() / 1024.0, 1) + " KB", 20, 130);
    tft.drawString(String(SYS_FLASH) + ": " + String(ESP.getFlashChipRealSize() / (1024.0 * 1024.0), 1) + " MB", 20, 130 + lineSpacing);
    tft.drawString(String(SYS_CPU) + ": " + String(ESP.getCpuFreqMHz()) + " MHz", 20, 130 + (lineSpacing * 2));

    // 5. Version Info (Moved from Start Menu to here)
    tft.setTextDatum(BC_DATUM);
    tft.drawString("NetTime OS v2.1.0-beta (2026)", 160, 200);

    // Cleanup
    tft.unloadFont();
    tft.setTextDatum(TL_DATUM);  // Restore default alignment
  }
};
#endif