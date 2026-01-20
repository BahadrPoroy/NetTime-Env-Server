#ifndef TOUCH_MANAGER_H
#define TOUCH_MANAGER_H

#include <TFT_eSPI.h>

class TouchManager {
  public:
    // Calibration data from your specific hardware setup
    uint16_t calData[5] = {406, 3422, 380, 3339, 1}; 

    void begin(TFT_eSPI &tft) {
      tft.setTouch(calData);
    }

    // Returns true if the screen is being touched and updates x, y coordinates
    bool isPressed(TFT_eSPI &tft, uint16_t &x, uint16_t &y) {
      return tft.getTouch(&x, &y);
    }
};

#endif