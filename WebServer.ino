void broadcastData() {
  char dataBuffer[17];
  // Alıcılar sadece ilk 5 karakteri (HH:MM) okuyorsa bu format çalışacaktır.
  // Eğer alıcı kodun split('|') yapabiliyorsa TEMP verisini de kullanabilir.
  sprintf(dataBuffer, "%02d:%02d|%02d/%02d/%04d", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_mday, timeinfo.tm_mon + 1, timeinfo.tm_year + 1900);
  
  IPAddress broadcastIP = WiFi.localIP();
  broadcastIP[3] = 255;
  udp.beginPacket(broadcastIP, udpPort);
  udp.write(dataBuffer);
  udp.endPacket();
}

void updateFirebase() {
  float t = DHT.temperature; 
  float h = DHT.humidity;

  if (isnan(t) || isnan(h)) {
    Serial.println("Sensor reading fault!");
    return;
  }

  // 1. Saat ve Tarih Formatlama (handleData içindeki gibi)
  char timeBuff[9];
  char dateBuff[11];
  sprintf(timeBuff, "%02d:%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
  sprintf(dateBuff, "%02d/%02d/%04d", timeinfo.tm_mday, timeinfo.tm_mon + 1, timeinfo.tm_year + 1900);

  // 2. Verileri Firebase'e Gönder
  bool success = true;
  
  // Önemli: JavaScript tarafındaki ID'lerle uyumlu yolları (path) kullanıyoruz
  if (!Firebase.setFloat(firebaseData, "/NetTime/sicaklik", t)) success = false;
  if (!Firebase.setFloat(firebaseData, "/NetTime/nem", h)) success = false;
  if (!Firebase.setString(firebaseData, "/NetTime/son_guncelleme", String(timeBuff))) success = false;
  if (!Firebase.setString(firebaseData, "/NetTime/tarih", String(dateBuff))) success = false;

  if (success) {
    Serial.println("Firebase successfully updated.");
  } else {
    Serial.print("Firebase Error: ");
    Serial.println(firebaseData.errorReason());
  }
}