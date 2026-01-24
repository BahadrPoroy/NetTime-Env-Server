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
#define WEATHER_BTN "Hava Durumu"
#define SETTINGS_TITLE "AYARLAR"
#define SYSTEM_TITLE "SİSTEM"
#define SYSTEM_BTN "Sistem"
#define TXT_RESTART "Yeniden\nBaşlat"
#define TXT_CONSOLE "Sistem Paneli"
#define TXT_NET "Ağ"
#define SYS_RAM   "Boş Bellek"
#define SYS_FLASH "Flash Boyutu"
#define SYS_CPU   "İşlemci Hızı"
#define SYS_UPDATE "Sistem Güncelleniyor"
#define TXT_UPDATE_NO_POWER "Gücü Kapatmayın"
#define TXT_UPDATE_SUCCESS "Güncelleme başarılı\nYeniden Başlatılıyor..."
#define SYS_REBOOTING "YENİDEN\nBAŞLATILIYOR..."
#define SYS_STORAGE "Boş Depolama"
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
#define WEATHER_BTN "Weather"
#define SETTINGS_TITLE "SETTINGS"
#define SYSTEM_TITLE "SYSTEM"
#define SYSTEM_BTN "System"
#define TXT_RESTART "Restart"
#define TXT_CONSOLE "Console"
#define TXT_NET "Network"
#define SYS_RAM   "Memory Free"
#define SYS_FLASH "Flash Storage"
#define SYS_CPU   "CPU Freq"
#define SYS_UPDATE "System Update"
#define TXT_UPDATE_NO_POWER "Don't power off"
#define TXT_UPDATE_SUCCESS "SUCCESS! REBOOTING..."
#define SYS_REBOOTING "REBOOTING..."
#define SYS_STORAGE "Free Storage"
const char* days[] = { "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday" };
#endif

#endif