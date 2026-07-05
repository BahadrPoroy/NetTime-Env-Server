/*
 * Master Server V2.8.5-pre-alpha Config
 * Branch: feature/tft-upgrade-v2
 */
#undef TFT_CS
#undef TFT_DC
#undef TFT_RST
#undef SD_CS

// TFT & Touch Pins
#define TFT_CS 5   
#define TFT_DC 2    
#define TFT_RST 4 
#define TFT_LED 21  //(Backlight PWM)

#define TOUCH_CS 15

#define SD_CS 22

// SPI Pins (Common for TFT & Touch)
#define SPI_SCK 18
#define SPI_MISO 19
#define SPI_MOSI 23

// Sensor Pins
#define DHTPIN 27

#ifndef DHTLIB_OK
#define DHTLIB_OK 0

// DHT Type
#define DHTTYPE DHT22

//Definition of Pages
enum Page {
  NONE,
  HOME_PAGE,      // Main dashboard
  DESKTOP_PAGE,   // Desktop for Page Icons
  WEATHER_PAGE,   // Weather Sensors
  SYSTEM_PAGE,    // System Properties and device info
  FEEDER_PAGE,    // Feeding System Controls
  SETTINGS_PAGE,  // Settings Page for future updates
  // Sub Pages of Settings Page
  LANGUAGE_SETTINGS,
  DISPLAY_SETTINGS,
  FEEDER_SETTINGS
};

enum Language {
  ENGLISH,
  TURKCE
};

static constexpr int MAX_PWM = 4095;  //Maximum PWM value used

// Definition of screen height & width values
static constexpr int SCREEN_WIDTH = 320;
static constexpr int SCREEN_HEIGHT = 240;

// Definition of icon height & width values
static constexpr int ICON_W = 32;
static constexpr int ICON_H = 32;
static constexpr int TRAY_ICON_H = 16;
static constexpr int TRAY_ICON_W = 16;

// Definition of static areas height & width values
static constexpr int TASKBAR_HEIGHT = 35;
static constexpr int HEADER_HEIGHT = 30;

#endif