void setupWebServer() {
  server.on("/", handleRoot);
  server.on("/data", handleData);
  server.begin();
}

void handleRoot() {
  String html = "<!DOCTYPE html><html><head><meta charset='UTF-8'>";
  html += "<title>NetTime Dashboard</title>";
  html += "<style>body{font-family:sans-serif;background:#1a1a2e;color:white;text-align:center;padding-top:50px;}";
  html += ".card{background:#16213e;display:inline-block;padding:20px;border-radius:15px;box-shadow:0 4px 10px rgba(0,0,0,0.3);}";
  html += ".val{font-size:2.5rem;color:#e94560;font-weight:bold;}</style></head><body>";
  
  html += "<div class='card'><h2>NetTime Server</h2>";
  // Dil değişkenleri burada kullanılıyor
  html += "<p>" + String(TXT_TIME) + ": <br><span id='t' class='val'>--:--:--</span></p>";
  html += "<p>" + String(WEB_TEMP) + ": <br><span id='tp' class='val'>--</span>°C</p></div>";

  html += "<script>setInterval(()=>{fetch('/data').then(r=>r.json()).then(d=>{";
  html += "document.getElementById('t').innerText=d.time;";
  html += "document.getElementById('tp').innerText=d.temp;";
  html += "});},1000);</script></body></html>";
  
  server.send(200, "text/html", html);
}

void handleData() {
  char timeBuff[9];
  // Format current time as HH:MM:SS
  sprintf(timeBuff, "%02d:%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
  
  // Create JSON response for web dashboard
  String json = "{";
  json += "\"time\":\"" + String(timeBuff) + "\",";
  json += "\"temp\":" + String(DHT.temperature);
  json += "}";
  
  server.send(200, "application/json", json); // Send JSON headers and data
}

void broadcastData() {
  char dataBuffer[16];
  // Alıcılar sadece ilk 5 karakteri (HH:MM) okuyorsa bu format çalışacaktır.
  // Eğer alıcı kodun split('|') yapabiliyorsa TEMP verisini de kullanabilir.
  sprintf(dataBuffer, "%02d:%02d|%02d", timeinfo.tm_hour, timeinfo.tm_min, DHT.temperature);
  
  IPAddress broadcastIP = WiFi.localIP();
  broadcastIP[3] = 255;
  udp.beginPacket(broadcastIP, udpPort);
  udp.write(dataBuffer);
  udp.endPacket();
}