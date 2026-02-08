#ifndef STRUCTS_H
#define STRUCTS_H

#include <Arduino.h>

struct WeatherData {
  float temp;
  int humidity;
  String icon;
  String description;
  bool updated = false;
};

#endif