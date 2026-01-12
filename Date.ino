void displayDatePage() {
  int16_t x1, y1;
  uint16_t w, h;
  char dateBuffer[11];

  display.clearDisplay();
  display.setTextSize(2);

  sprintf(dateBuffer, "%02d/%02d/%04d", timeinfo.tm_mday, timeinfo.tm_mon + 1, timeinfo.tm_year + 1900);
  const char* currentDay = days[timeinfo.tm_wday];  // language.h'den gelir

  // Row 1: Date
  display.getTextBounds(dateBuffer, 0, 0, &x1, &y1, &w, &h);
  display.setCursor((SCREEN_WIDTH - w) / 2, 10);
  display.print(dateBuffer);

  // Row 2: Day Name
  display.getTextBounds(currentDay, 0, 0, &x1, &y1, &w, &h);
  display.setCursor((SCREEN_WIDTH - w) / 2, 34);
  display.print(currentDay);
  drawWiFiIcon(WiFi.RSSI());
  display.display();
}