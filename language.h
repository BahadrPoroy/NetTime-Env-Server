#ifndef LANGUAGE_H
#define LANGUAGE_H

// Dil seçimi: 0 = English, 1 = Turkish
#define SELECTED_LANG 1

#if SELECTED_LANG == 1
// --- Türkçe ---
#define WEB_TEMP "SICAKLIK"
#define TXT_TEMP "Sıcaklık"
#define WEB_HUM "NEM"
#define TXT_HUM "Nem"
#define TXT_DATE "Tarih"
#define TXT_TIME "Saat"
#define WEATHER_TITLE "HAVA DURUMU"
#define SETTINGS_TITLE "AYARLAR"
#define ABOUT_TITLE "HAKKINDA"
#define TXT_RESTART "Yeniden\nBaşlat"
const char* days[] = { "Pazar", "Pazartesi", "Salı", "Çarşamba", "Perşembe", "Cuma", "Cumartesi" };
#else
// --- English ---
#define WEB_TEMP "TEMPERATURE"
#define TXT_TEMP "Tempreture"
#define WEB_HUM "HUMIDITY"
#define TXT_HUM "Humidity"
#define TXT_DATE "Date"
#define TXT_TIME "Time"
#define WEATHER_TITLE "WEATHER"
#define SETTINGS_TITLE "SETTINGS"
#define ABOUT_TITLE "ABOUT"
#define TXT_RESTART "Restart"
const char* days[] = { "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday" };
#endif

#endif