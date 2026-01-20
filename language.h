#ifndef LANGUAGE_H
#define LANGUAGE_H

// Dil seçimi: 0 = English, 1 = Turkish
#define SELECTED_LANG 1

#if SELECTED_LANG == 1
// --- Türkçe ---
#define WEB_TEMP "SICAKLIK"
#define TXT_TEMP "SIC"
#define WEB_HUM "NEM"
#define TXT_HUM "NEM"
#define TXT_DATE "TARIH"
#define TXT_TIME "SAAT"
#define WEATHER_TITLE "HAVA DURUMU"
#define SETTINGS_TITLE "AYARLAR"
#define ABOUT_TITLE "HAKKINDA"
const char* days[] = { "Pazar", "Pazartesi", "Sali", "Carsamba", "Persembe", "Cuma", "Cumartesi" };
#else
// --- English ---
#define WEB_TEMP "TEMPERATURE"
#define TXT_TEMP "TEMP"
#define WEB_HUM "HUMIDITY"
#define TXT_HUM "HUM"
#define TXT_DATE "DATE"
#define TXT_TIME "TIME"
#define WEATHER_TITLE "WEATHER"
#define SETTINGS_TITLE "SETTINGS"
#define ABOUT_TITLE "ABOUT"
const char* days[] = { "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday" };
#endif

#endif