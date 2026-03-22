#ifndef TIME_MANAGER_H
#define TIME_MANAGER_H

#include <time.h>
#include <WiFiUdp.h>

class TimeManager {
private:
  struct tm timeinfo;
  const char* ntpServer = "pool.ntp.org";
  const long gmtOffset_sec = 10800;  // UTC+3 for Turkey
  const int daylightOffset_sec = 0;

public:

  long getTimestamp() {
    return (long)time(nullptr);
  }

  void begin() {
    // Initialize NTP and sync time
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    updateInternalTime();
  }

  void updateInternalTime() {
    if (!getLocalTime(&timeinfo)) {
      Serial.println("Failed to obtain time");
      return;
    }
  }

  // Returns time in HH:MM:SS format
  String getFormattedTime() {
    updateInternalTime();
    char timeBuffer[9];
    sprintf(timeBuffer, "%02d:%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
    return String(timeBuffer);
  }

  // Returns date in DD/MM/YYYY format
  String getFormattedDate() {
    updateInternalTime();
    char dateBuffer[11];
    sprintf(dateBuffer, "%02d/%02d/%04d", timeinfo.tm_mday, timeinfo.tm_mon + 1, timeinfo.tm_year + 1900);
    return String(dateBuffer);
  }

  int getHour() {
    return timeinfo.tm_hour;
  }
  int getMinute() {
    return timeinfo.tm_min;
  }
  int getSecond() {
    return timeinfo.tm_sec;
  }

  // Returns day of the week (e.g., Monday, Sunday)
  String getDayName() {
    updateInternalTime();
    // tm_wday returns 0 for Sunday, 1 for Monday, etc.
    return String(dayNames[timeinfo.tm_wday][settingsData.language]);
}
};

#endif