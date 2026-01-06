void displayTimePage() {
  int16_t x1, y1;
  uint16_t w, h;
  char timeBuffer[9];

  if (getLocalTime(&timeinfo)) {
    display.clearDisplay();
    display.setFont(&FreeSansBold12pt7b);
    display.setTextSize(1);

    sprintf(timeBuffer, "%02d:%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);

    display.getTextBounds(timeBuffer, 0, 0, &x1, &y1, &w, &h);
    int xPos = (SCREEN_WIDTH - w) / 2;
    int yPos = (SCREEN_HEIGHT + h) / 2;

    display.setCursor(xPos, yPos);
    display.print(timeBuffer);

    display.setFont();

    drawWiFiIcon(WiFi.RSSI());
    display.display();
  }
}