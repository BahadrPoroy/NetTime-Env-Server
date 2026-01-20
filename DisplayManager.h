#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include <TFT_eSPI.h>
#include "language.h"

class DisplayManager {
public:

  void init(TFT_eSPI &tft) {
    tft.init();
    tft.setRotation(3);
    tft.fillScreen(TFT_BLACK);
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
    tft.setTextSize(1);
    tft.setTextDatum(MC_DATUM);
    tft.drawString("RESTART", 52, 187);

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

  void resetTextSettings(TFT_eSPI &tft) {
    tft.setTextSize(1);
    tft.setTextDatum(TL_DATUM);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
  }

  void drawStaticUI(TFT_eSPI &tft) {
    // 1. Higher Taskbar area (from y:205 to 240)
    tft.fillRect(0, 205, 320, 35, 0x0112);
    tft.drawFastHLine(0, 205, 320, 0x319F);  // Taskbar border

    // 2. Resized Start Button (Centered in the new bar)
    tft.fillRoundRect(2, 210, 45, 26, 4, 0x02D7);
    tft.fillCircle(24, 223, 7, TFT_WHITE);  // Win Icon
  }

  void drawWelcomeScreen(TFT_eSPI &tft) {
    tft.fillScreen(0x0000);  // Black
    tft.setTextColor(TFT_SKYBLUE);
    tft.setTextDatum(MC_DATUM);  // Middle center

    // Fantasy Boot Text (We can replace this with an image array later)
    tft.drawString("NETTIME OS", 160, 50, 4);
    tft.drawSmoothArc(160, 120, 50, 45, 0, 360, TFT_SKYBLUE, TFT_BLACK);

    tft.setTextSize(1);
    tft.drawString("V2.0.0 PREMIUM EDITION", 160, 210, 2);
    delay(2000);  // Give time to see the "Logo"
    tft.fillScreen(TFT_BLACK);
  }

  void drawHeader(TFT_eSPI &tft, const char *title, uint16_t bgColor, uint16_t txtColor) {
    tft.fillRect(0, 0, 320, 30, bgColor);
    tft.setTextSize(1);
    tft.setTextDatum(MC_DATUM);
    tft.setTextColor(txtColor, bgColor);
    tft.drawString(title, 160, 15, 2);
  }

  void updateWeather(TFT_eSPI &tft, float temp, float hum) {

    // 1. Set stable font settings
    tft.setTextSize(2);
    tft.setTextDatum(TL_DATUM);

    // Temperature (Anti-flicker: providing background color)
    tft.setTextColor(0x04DF, TFT_BLACK);
    tft.drawString(String(TXT_TEMP) + ": " + String(temp, 1) + " C  ", 10, 35);

    // Humidity
    tft.setTextColor(0x04DF, TFT_BLACK);
    tft.drawString(String(WEB_HUM) + ": %" + String(hum, 0) + "   ", 200, 35);

    resetTextSettings(tft);  // Always reset after custom sizing
  }

  void updateClock(TFT_eSPI &tft, String timeStr, String dateStr) {
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
};
#endif