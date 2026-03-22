#ifndef STRUCTS_H
#define STRUCTS_H

#include <Arduino.h>

struct WeatherData {
  float temp;
  int humidity;
  String icon;
  String description;
  unsigned long sunrise;  // Unix timestamp
  unsigned long sunset;   // Unix timestamp
  bool updated = false;
};

struct SettingsData {
  int feederStart;
  int feederEnd;
  int manBright;
  int dayBright;
  int nightBright;
  bool isAdaptive;
  int language;
};

#endif