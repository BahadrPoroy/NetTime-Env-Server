/*
 * Master Server V2.0.0-beta Config
 * Branch: feature/tft-upgrade-v2
 */
#undef TFT_CS
#undef TFT_DC
#undef TFT_RST
#undef SD_CS

// TFT & Touch Pins
#define TFT_CS    15 // D8
#define TFT_DC     0 // D3
#define TFT_RST   -1 // Disabled
#define TFT_LED    4 // D2 (Backlight PWM)

#define TOUCH_CS   2 // D4

#define SD_CS    16

// SPI Pins (Common for TFT & Touch)
#define SPI_SCK   14 // D5
#define SPI_MISO  12 // D6
#define SPI_MOSI  13 // D7

// Sensor Pins
#define DHTPIN     5 // D1

#ifndef DHTLIB_OK
#define DHTLIB_OK 0
#endif