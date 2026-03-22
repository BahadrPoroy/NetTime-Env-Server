#ifndef THEMES_H
#define THEMES_H

#include <Arduino.h>

struct Theme {
  uint16_t bg_color;
  uint16_t active_color;
  uint16_t passive_color;
  uint16_t text_color;
  uint16_t border_color;
  uint16_t taskbar_color;
};

// (Default) Turquoise Theme
const Theme TurquoiseTheme = {
  0x0000,  // Background Color (black)
  0x04DF,  // Active Color (turquoise)
  0x0063,  // Passive & Alternative Background (dark blue)
  0xFFFF,  // Alternative Label Color (white)
  0x04DF,  // Border (turquoise)
  0x0112   //Taskbar (blue)
};

extern const Theme* currentTheme;

// Macros (For easy calling by names)
#define ACTIVE_COLOR (currentTheme->active_color)
#define PASSIVE_COLOR (currentTheme->passive_color)
#define BG_COLOR (currentTheme->bg_color)
#define BG_COLOR_ALT PASSIVE_COLOR
#define LBL_COLOR ACTIVE_COLOR
#define LBL_COLOR_ALT (currentTheme->text_color)
#define TSKBAR_COLOR (currentTheme->taskbar_color)

#endif