/*
 * Master Server V2.8.5-pre-alpha Config
 * Branch: feature/tft-upgrade-v2
 */
#define NEXTION_RX 16
#define NEXTION_TX 17

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


#endif