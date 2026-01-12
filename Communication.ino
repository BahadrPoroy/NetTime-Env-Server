void setupWebServer() {
  server.on("/", handleRoot);
  server.on("/data", handleData);
  server.begin();
}

void handleRoot() {
  String html = "<!DOCTYPE html><html><head><meta charset='UTF-8'>";
  html += "<title>NetTime Dashboard</title>";
  html += "<style>body{font-family:sans-serif;background:#000000;color:#000000;text-align:center;padding-top:50px;}";
  html += ".card{background:#c0c0c0;display:inline-block;padding:20px;border-radius:15px;box-shadow:0 4px 10px rgba(0,0,0,0.3); min-width:300px;}";
  html += ".val{font-size:2rem;color:#ff0000;font-weight:bold;}";  // Boyutu biraz küçülttük ki yan yana sığsın
  html += ".row{display:flex; justify-content:space-around; margin-top:20px;}";
  html += "</style></head><body>";

  html += "<div class='card'><h2>NetTime Server</h2>";

  // 1. Tarih
  html += "<p>" + String(TXT_DATE) + ":<br><span id='date' class='val'>--/--/--</span></p>";

  // 2. Saat
  html += "<p>" + String(TXT_TIME) + ":<br><span id='t' class='val'>--:--:--</span></p>";

  // 3. Sıcaklık ve Nem (Yan yana)
  html += "<div class='row'>";
  html += "  <div><p>" + String(WEB_TEMP) + ":<br><span id='tp' class='val'>--</span>°C</p></div>";
  html += "  <div><p>" + String(WEB_HUM) + ":<br><span id='hm' class='val'>--</span>%</p></div>";
  html += "</div>";

  html += "</div>";  // Card sonu

  // JavaScript Güncelleme
  html += "<script>setInterval(()=>{fetch('/data').then(r=>r.json()).then(d=>{";
  html += "document.getElementById('date').innerText=d.date; ";  // Tarih güncelleme
  html += "document.getElementById('t').innerText=d.time;";
  html += "document.getElementById('tp').innerText=d.temp;";
  html += "document.getElementById('hm').innerText=d.humid;";  // Nem güncelleme
  html += "});},1000);</script></body></html>";

  server.send(200, "text/html", html);
}

void handleData() {
  char timeBuff[9];
  char dateBuff[11];
  // Format current time as HH:MM:SS
  sprintf(timeBuff, "%02d:%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
  sprintf(dateBuff, "%02d/%02d/%04d", timeinfo.tm_mday, timeinfo.tm_mon + 1, timeinfo.tm_year + 1900);
  // Create JSON response for web dashboard
  String json = "{";
  json += "\"date\":\"" + String(dateBuff) + "\",";
  json += "\"time\":\"" + String(timeBuff) + "\",";
  json += "\"temp\":\"" + String(DHT.temperature) + "\",";
  json += "\"humid\":\"" + String(DHT.humidity) + "\"";
  json += "}";

  server.send(200, "application/json", json);  // Send JSON headers and data
}

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

  char timeBuff[9];
  char dateBuff[11];
  sprintf(timeBuff, "%02d:%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
  sprintf(dateBuff, "%02d/%02d/%04d", timeinfo.tm_mday, timeinfo.tm_mon + 1, timeinfo.tm_year + 1900);

  // Get Unix Timestamp (For the upcoming "Fluid Clock" feature)
  time_t now = time(nullptr);

  // Push Data to Firebase
  bool success = true;

  if (!Firebase.setFloat(firebaseData, "/NetTime/sicaklik", t)) success = false;
  if (!Firebase.setFloat(firebaseData, "/NetTime/nem", h)) success = false;
  if (!Firebase.setString(firebaseData, "/NetTime/son_guncelleme", String(timeBuff))) success = false;
  if (!Firebase.setString(firebaseData, "/NetTime/tarih", String(dateBuff))) success = false;
  // New: Adding the timestamp for our JavaScript logic
  if (!Firebase.setInt(firebaseData, "/NetTime/timestamp", now)) success = false;
  if (success) {
    Serial.println("Firebase successfully updated.");
  } else {
    Serial.print("Firebase Error: ");
    Serial.println(firebaseData.errorReason());
  }
}