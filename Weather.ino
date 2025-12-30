void displayWeatherPage() {
  int16_t x1, y1;
  uint16_t w, h;
  char buffer[20];

  display.clearDisplay();
  display.setTextSize(2);

  int currentTemp = DHT.temperature;
  int currentHum = DHT.humidity;

  // Row 1: Temperature (Örn: SICAKLIK: 25°C)
  sprintf(buffer, "%s: %02d%cC", TXT_TEMP, currentTemp, 247);
  display.getTextBounds(buffer, 0, 0, &x1, &y1, &w, &h);
  display.setCursor((SCREEN_WIDTH - w) / 2, 16);
  display.print(buffer);

  // Row 2: Humidity (Örn: NEM: %45)
  sprintf(buffer, "%s: %%%02d", TXT_HUM, currentHum);
  display.getTextBounds(buffer, 0, 0, &x1, &y1, &w, &h);
  display.setCursor((SCREEN_WIDTH - w) / 2, 40);
  display.print(buffer);
  drawWiFiIcon(WiFi.RSSI());
  display.display();
}