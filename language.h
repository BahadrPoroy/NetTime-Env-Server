#ifndef LANGUAGE_H
#define LANGUAGE_H

// language selection is reading from database: 0 = English, 1 = Turkish
const char* const langTable[][2] = {
  { "TEMPERATURE", "SICAKLIK" },                                                // 0
  { "Temperature", "Sıcaklık" },                                                // 1
  { "HUMIDITY", "NEM" },                                                        // 2
  { "Humidity", "Nem" },                                                        // 3
  { "Date", "Tarih" },                                                          // 4
  { "Time", "Saat" },                                                           // 5
  { "WEATHER", "HAVA DURUMU" },                                                 // 6
  { "Weather", "Hava Durumu" },                                                 // 7
  { "SETTINGS", "AYARLAR" },                                                    // 8
  { "Settings", "Ayarlar" },                                                    // 9
  { "SYSTEM", "SİSTEM" },                                                       // 10
  { "HOME", "ANA SAYFA" },                                                      // 11
  { "Home", "Ana Sayfa" },                                                      // 12
  { "System", "Sistem" },                                                       // 13
  { "Restart", "Yeniden\nBaşlat" },                                             // 14
  { "Console", "Sistem Paneli" },                                               // 15
  { "Network", "Ağ" },                                                          // 16
  { "Memory Free", "Boş Bellek" },                                              // 17
  { "Flash Storage", "Flash Boyutu" },                                          // 18
  { "CPU Freq", "İşlemci Hızı" },                                               // 19
  { "System Update", "Sistem Güncelleniyor" },                                  // 20
  { "Don't power off", "Gücü Kapatmayın" },                                     // 21
  { "SUCCESS! REBOOTING...", "Güncelleme başarılı\nYeniden Başlatılıyor..." },  // 22
  { "REBOOTING...", "YENİDEN\nBAŞLATILIYOR..." },                               // 23
  { "Free Storage", "Boş Depolama" },                                           // 24
  { "Fed", "Yemlendi" },                                                        // 25
  { "Waiting", "Bekleniyor" },                                                  // 26
  { "Feeder Error", "Yemleyici Hatası" },                                       // 27
  { "Feeder", "Yemleyici" },                                                    // 28
  { "FEEDER SYSTEM", "YEMLEME SİSTEMİ" },                                       // 29
  { "INDOOR", "ODA HAVASI" },                                                   // 30
  { "OUTDOOR", "DIŞ HAVA DURUMU" },                                             // 31
  { "INDOOR TEMP", "İÇ" },                                                      // 32
  { "OUTDOOR TEMP", "DIŞ" },                                                    // 33
  { "Updating...", "Güncelleniyor..." },                                         // 34
  { "Adaptive Brightness", "Adaptif Parlaklık" }                                //35
};

// --- 2. Week day names ---
const char* const dayNames[7][2] = {
  { "Sunday", "Pazar" }, { "Monday", "Pazartesi" }, { "Tuesday", "Salı" }, { "Wednesday", "Çarşamba" }, { "Thursday", "Perşembe" }, { "Friday", "Cuma" }, { "Saturday", "Cumartesi" }
};

// --- 3. AKILLI MAKROLAR (Kodun bozulmaması için) ---
// Old names are compatible with new table provide of these macros 
// settingsData.language, EN = 0, TR = 1 for now.

#define WEB_TEMP langTable[0][settingsData.language]
#define TXT_TEMP langTable[1][settingsData.language]
#define WEB_HUM langTable[2][settingsData.language]
#define TXT_HUM langTable[3][settingsData.language]
#define TXT_DATE langTable[4][settingsData.language]
#define TXT_TIME langTable[5][settingsData.language]
#define WEATHER_TITLE langTable[6][settingsData.language]
#define WEATHER_BTN langTable[7][settingsData.language]
#define SETTINGS_TITLE langTable[8][settingsData.language]
#define SETTINGS_BTN langTable[9][settingsData.language]
#define SYSTEM_TITLE langTable[10][settingsData.language]
#define HOME_TITLE langTable[11][settingsData.language]
#define HOME_BTN langTable[12][settingsData.language]
#define SYSTEM_BTN langTable[13][settingsData.language]
#define TXT_RESTART langTable[14][settingsData.language]
#define TXT_CONSOLE langTable[15][settingsData.language]
#define TXT_NET langTable[16][settingsData.language]
#define SYS_RAM langTable[17][settingsData.language]
#define SYS_FLASH langTable[18][settingsData.language]
#define SYS_CPU langTable[19][settingsData.language]
#define SYS_UPDATE langTable[20][settingsData.language]
#define TXT_UPDATE_NO_POWER langTable[21][settingsData.language]
#define TXT_UPDATE_SUCCESS langTable[22][settingsData.language]
#define SYS_REBOOTING langTable[23][settingsData.language]
#define SYS_STORAGE langTable[24][settingsData.language]
#define TXT_FED langTable[25][settingsData.language]
#define TXT_WAIT langTable[26][settingsData.language]
#define TXT_ERR langTable[27][settingsData.language]
#define TXT_FEEDER langTable[28][settingsData.language]
#define FEEDER_BTN langTable[29][settingsData.language]
#define FEEDER_TITLE langTable[29][settingsData.language]
#define TXT_INDOOR langTable[30][settingsData.language]
#define TXT_OUTDOOR langTable[31][settingsData.language]
#define TXT_INDOOR_TEMP langTable[32][settingsData.language]
#define TXT_OUTDOOR_TEMP langTable[33][settingsData.language]
#define TXT_UPDATING langTable[34][settingsData.language]
#define OPT_ADAPTIVE langTable[35][settingsData.language]

#endif