#include "config.h"
#include "structs.h"
#include <DHT.h>
#include <DHT_U.h>
#include "NetworkManager.h"
#include "TimeManager.h"

// FreeRTOS Kütüphaneleri
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

// --- Global Nesneler ---
extern SettingsData settingsData;
SettingsData settingsData;
DHT DHT(DHTPIN, DHTTYPE);
NetBoxManager netBox;
TimeManager timeBox;

// --- Global Veriler ---
float currentTemp, currentHum;
bool isFed = false;
String feederStatus = "IDLE";
long lastFedTime = 0;
String currentPage = "main";  // Nextion üzerinde aktif olan sayfa adı

// Nextion Seri Port
#define NextionSerial Serial2
SemaphoreHandle_t xNextionMutex;          // Seri port için kilit (Mutex)
TaskHandle_t xUIUpdateTaskHandle = NULL;  // UI Görevini uzaktan tetiklemek için Handle

// --- Fonksiyon: Nextion'a Güvenli Yazma (Mutex Korumalı) ---
void sendNextionCommand(String cmd) {
  if (xSemaphoreTake(xNextionMutex, pdMS_TO_TICKS(500))) {
    NextionSerial.print(cmd);
    NextionSerial.write(0xFF);
    NextionSerial.write(0xFF);
    NextionSerial.write(0xFF);
    xSemaphoreGive(xNextionMutex);
  }
}

// --- Yardımcı: Hava Durumu İkon ID ---
int getWeatherPicId(String icon) {
  if (icon == "01d") return 7;
  if (icon == "01n") return 8;
  if (icon == "02d") return 9;
  if (icon == "02n") return 10;
  if (icon == "03d") return 11;
  if (icon == "03n") return 12;
  if (icon == "04d") return 13;
  if (icon == "04n") return 14;
  if (icon == "09d") return 15;
  if (icon == "09n") return 16;
  if (icon == "10d") return 17;
  if (icon == "10n") return 18;
  if (icon == "11d") return 19;
  if (icon == "11n") return 20;
  if (icon == "13d") return 21;
  if (icon == "13n") return 22;
  if (icon == "50d") return 23;
  if (icon == "50n") return 24;
  return 0;
}

// --- GÖREVLER ---

// 1. Sensör Okuma
void TaskSensor(void *pvParameters) {
  for (;;) {
    currentTemp = DHT.readTemperature();
    currentHum = DHT.readHumidity();
    vTaskDelay(pdMS_TO_TICKS(2000));
  }
}

// 2. Nextion Komut İşleme
void TaskNextion(void *pvParameters) {
  String nextionCommandBuffer = "";
  for (;;) {
    while (NextionSerial.available() > 0) {
      char inChar = (char)NextionSerial.read();
      if (inChar == '\n') {
        nextionCommandBuffer.trim();
        if (nextionCommandBuffer == "FEED") {
          netBox.broadcastUDP("FEED_NOW");
          lastFedTime = timeBox.getTimestamp();
        }
        // Sayfa değiştiğinde Nextion'dan gelen PAGE:sayfaAdi verisi ayrıştırılır
        else if (nextionCommandBuffer.startsWith("PAGE:")) {
          currentPage = nextionCommandBuffer.substring(5);  // "PAGE:" sonrasını al
          currentPage.trim();

          if (xUIUpdateTaskHandle != NULL) {
            // UI Güncelleme görevini ANINDA uyandır!
            xTaskNotifyGive(xUIUpdateTaskHandle);
          }
        } else if (nextionCommandBuffer == "PAGE_CHANGED" || nextionCommandBuffer.startsWith("PAGE")) {
          if (xUIUpdateTaskHandle != NULL) {
            xTaskNotifyGive(xUIUpdateTaskHandle);
          }
        }
        nextionCommandBuffer = "";
      } else if (inChar != '\r') {
        nextionCommandBuffer += inChar;
      }
    }
    vTaskDelay(pdMS_TO_TICKS(50));
  }
}

// 3. Ağ, Firebase ve OTA
void TaskNetworkAndLogic(void *pvParameters) {
  for (;;) {
    netBox.handleOTA();  // OTA desteği

    if (WiFi.status() == WL_CONNECTED) {
      netBox.updateFirebase(currentTemp, currentHum, timeBox.getFormattedTime(),
                            timeBox.getFormattedDate(), timeBox.getTimestamp(), isFed, lastFedTime);
      netBox.handleFeederNetwork(feederStatus, isFed, lastFedTime, timeBox.getTimestamp());
      netBox.handleOpenWeather();
    }

    if (!isFed && (timeBox.getHour() <= settingsData.feederEnd && timeBox.getHour() >= settingsData.feederStart)) {
      netBox.broadcastUDP("FEED_NOW");
    }
    vTaskDelay(pdMS_TO_TICKS(100));  // Hızlı tepki için OTA kontrolü
  }
}

// 4. UI Güncellemeleri (Dış Ortam ve Feeder Durumu dahil)
void TaskUIUpdate(void *pvParameters) {
  for (;;) {
    // --- DIŞ ORTAM HAVA DURUMU (OpenWeather) ---
    if (netBox.currentWeather.updated) {
      float outTemp = netBox.currentWeather.temp;
      float outHum = netBox.currentWeather.humidity;

      if (netBox.currentWeather.icon.length() > 0) {
        sendNextionCommand("Weather.pic=" + String(getWeatherPicId(netBox.currentWeather.icon)));
      }
      sendNextionCommand("outTemp.txt=\"" + String(outTemp, 1) + " C\"");
      sendNextionCommand("outHum.txt=\"" + String(outHum, 1) + "%\"");
    }

    // --- İÇ ORTAM SENSÖR (DHT) ---
    sendNextionCommand("inTemp.txt=\"" + String(currentTemp, 1) + " C\"");
    sendNextionCommand("inHum.txt=\"" + String(currentHum, 1) + "%\"");

    // --- Feeder Durum Mantığı ---
    if (isFed) {
      sendNextionCommand("feeder.txt=\"Beslendi\"");
      sendNextionCommand("feeder.pco=2016");  // Yeşil
    } else if (timeBox.getHour() < settingsData.feederStart) {
      sendNextionCommand("feeder.txt=\"Bekleniyor\"");
      sendNextionCommand("feeder.pco=65504");  // Sarı
    } else if (timeBox.getHour() > settingsData.feederEnd) {
      sendNextionCommand("feeder.txt=\"HATA\"");
      sendNextionCommand("feeder.pco=63488");  // Kırmızı
    }

    // En fazla 10 saniye bekle VEYA sayfa değiştiğinde / bildirim geldiğinde ANINDA uyan!
    ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(10000));
  }
}

// 5. Saat
void TaskClock(void *pvParameters) {
  for (;;) {
    timeBox.updateInternalTime();
    sendNextionCommand("tTime.txt=\"" + timeBox.getFormattedHour() + "\"");
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

void setup() {
  Serial.begin(115200);
  NextionSerial.begin(9600, SERIAL_8N1, NEXTION_RX, NEXTION_TX);
  xNextionMutex = xSemaphoreCreateMutex();

  netBox.begin();
  timeBox.begin();
  DHT.begin();

  // Görevleri Başlat
  xTaskCreate(TaskSensor, "SensorTask", 2048, NULL, 1, NULL);
  xTaskCreate(TaskNextion, "NextionTask", 4096, NULL, 3, NULL);
  xTaskCreate(TaskNetworkAndLogic, "NetLogicTask", 8192, NULL, 2, NULL);
  // UI Güncelleme görevine tetiklenebilmesi için Handle (&xUIUpdateTaskHandle) atıyoruz
  xTaskCreate(TaskUIUpdate, "UIUpdateTask", 4096, NULL, 1, &xUIUpdateTaskHandle);
  xTaskCreate(TaskClock, "ClockTask", 2048, NULL, 2, NULL);
}

void loop() {
  vTaskDelete(NULL);
}