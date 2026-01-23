#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include <TFT_eSPI.h>
#include <SD.h>
#include <LittleFS.h>
#include "language.h"

// Font File Names
const char *FONT_8 = "ATR8.vlw";
const char *FONT_12 = "ATR12.vlw";
const char *FONT_16 = "ATR16.vlw";
const char *FONT_20 = "ATR20.vlw";
const char *FONT_24 = "ATR24.vlw";
const char *FONT_28 = "ATR28.vlw";
const char *FONT_32 = "ATR32.vlw";

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
  void
  drawStaticSplash(TFT_eSPI &tft) {
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
    // Menu Background (Dark Gray/Blue Win7 Style)
    tft.fillRect(2, 120, 100, 85, 0x10A2);
    tft.drawRect(2, 120, 100, 85, 0x319F);

    // Restart Button Area
    tft.fillRect(5, 175, 94, 25, 0x02D7);  // Blue button
    tft.setTextColor(TFT_WHITE);
    tft.loadFont(FONT_12);
    tft.setTextDatum(MC_DATUM);
    tft.drawString(String(TXT_RESTART), 52, 187);
    tft.unloadFont();

    // Status text or other info
    tft.setTextDatum(TL_DATUM);
    tft.drawString("System", 10, 140);
    tft.drawString("v2.0 Beta", 10, 155);

    isMenuOpen = true;
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
    tft.loadFont(FONT_24);
    tft.setTextDatum(MC_DATUM);
    tft.setTextColor(txtColor, bgColor);
    tft.drawString(title, 160, 15, 2);
    tft.unloadFont();
  }

  void updateWeather(TFT_eSPI &tft, float temp, float hum) {

    // 1. Set stable font settings
    tft.unloadFont();
    tft.loadFont(FONT_16);

    tft.setTextDatum(TL_DATUM);

    // Temperature (Anti-flicker: providing background color)
    tft.setTextColor(0x04DF, TFT_BLACK);
    tft.drawString(String(TXT_TEMP) + ": " + String(temp, 1) + " °C", 10, 35);

    // Humidity
    tft.setTextColor(0x04DF, TFT_BLACK);
    tft.drawString(String(TXT_HUM) + ": %" + String(hum, 0) + "   ", 200, 35);
    tft.unloadFont();
  }

  void
  updateClock(TFT_eSPI &tft, String timeStr, String dateStr) {
    tft.setTextDatum(TR_DATUM);  // Align to Right

    // Time on Taskbar (Top row of tray)
    tft.setTextColor(TFT_WHITE, 0x0112);
    tft.setTextSize(1);
    // Show only HH:MM for tray, keep it clean
    tft.drawString(timeStr.substring(0, 5), 305, 206, 2);

    // Date on Taskbar (Bottom row of tray)
    tft.setTextColor(TFT_LIGHTGREY, 0x0112);
    tft.drawString(dateStr, 315, 222, 1);

    tft.setTextDatum(TL_DATUM);  // Reset
  }

  void drawExpandedClock(TFT_eSPI &tft, String fullTime, String date, String day, String ssid, bool firstDraw = false) {
    // Semi-transparent look window
    if (firstDraw) {
      tft.fillRect(140, 65, 175, 140, 0x0112);
      tft.drawRect(140, 65, 175, 140, 0x319F);
    }

    tft.setTextDatum(MC_DATUM);

    // Full Time with Seconds (Large)
    tft.setTextColor(TFT_WHITE, 0x0112);
    tft.drawString(fullTime, 227, 90, 4);

    // Date (Medium)
    tft.setTextColor(TFT_WHITE, 0x0112);
    tft.drawString(date, 227, 125, 2);

    // Day Name (From language.h)
    tft.setTextColor(TFT_WHITE, 0x0112);
    tft.drawString(day, 227, 155, 2);

    tft.setTextColor(0x04DF, 0x0112);
    tft.drawString(ssid, 220, 185, 2);

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
      tft.setTextColor(TFT_ORANGE, TFT_BLACK);
      tft.setTextDatum(MC_DATUM);

      // Use Font 2 (Medium size) instead of Font 4
      tft.drawString("SYSTEM UPDATE", 160, 80, 2);

      // Draw progress bar frame
      tft.drawRect(60, 110, 200, 20, TFT_WHITE);
    }

    // Fill progress bar
    tft.fillRect(62, 112, (progress * 196) / 100, 16, TFT_ORANGE);

    // Small percentage text below the bar
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextDatum(MC_DATUM);
    tft.setTextSize(1);  // Set size to 1 for safety
    tft.drawString(String(progress) + "%  ", 160, 140, 2);

    tft.drawString("Don't power off", 160, 170, 2);
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

  void setupLittleFS() {
    if (!LittleFS.begin()) {
      Serial.println("LittleFS Mount Failed");
      return;
    }

    const char *fonts[] = { FONT_8, FONT_12, FONT_16, FONT_20, FONT_24, FONT_32 };

    for (const char *f : fonts) {
      if (!LittleFS.exists(f)) {
        Serial.printf("%s LittleFS'de yok, SD'den aranıyor...\n", f);

        File src = SD.open(f);
        if (!src) {
          Serial.printf("HATA: %s SD KARTTA BULUNAMADI! Lütfen kartı kontrol edin.\n", f);
          continue;
        }

        File dest = LittleFS.open(f, "w");
        if (dest) {
          size_t written = 0;
          while (src.available()) {
            written += dest.write(src.read());
          }
          dest.close();
          Serial.printf("BAŞARILI: %s kopyalandı (%d byte).\n", f, written);
        } else {
          Serial.println("HATA: LittleFS'e yazılamadı! Yer kalmamış olabilir.");
        }
        src.close();
      } else {
        Serial.printf("%s zaten LittleFS'de mevcut.\n", f);
      }
    }
  }
};
#endif