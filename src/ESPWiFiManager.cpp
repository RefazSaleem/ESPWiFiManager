#include "ESPWiFiManager.h"

ESPWiFiManager::ESPWiFiManager() : server(80) {}

void ESPWiFiManager::begin() {
  Serial.begin(115200);
  EEPROM.begin(eepromSize);
  loadStringsFromEEPROM();

  if (storedStrings[0] != "" && storedStrings[1] != "") {
    connectToWiFi(storedStrings[0], storedStrings[1], "1");
  }

  if (WiFi.status() != WL_CONNECTED && storedStrings[2] != "" && storedStrings[3] != "") {
    connectToWiFi(storedStrings[2], storedStrings[3], "2");
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("WiFi connected successfully.");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("Failed to connect to any WiFi network. Starting AP mode...");
    startAPMode();
  }

  server.on("/", HTTP_GET, [this]() { handleRoot(); });
  server.on("/", HTTP_POST, [this]() { handleRootPost(); });
  server.begin();
}

void ESPWiFiManager::handleClient() {
  server.handleClient();
}

void ESPWiFiManager::loadStringsFromEEPROM() {
  for (int i = 0; i < maxStrings; i++) {
    int startAddress = i * maxLength;
    char buffer[maxLength];
    for (int j = 0; j < maxLength; j++) {
      buffer[j] = EEPROM.read(startAddress + j);
    }
    storedStrings[i] = String(buffer);
  }
}

void ESPWiFiManager::saveStringToEEPROM(int index, const String& str) {
  int startAddress = index * maxLength;
  for (int i = 0; i < maxLength; i++) {
    if (i < str.length()) {
      EEPROM.write(startAddress + i, str[i]);
    } else {
      EEPROM.write(startAddress + i, 0);
    }
  }
  EEPROM.commit();
}

void ESPWiFiManager::connectToWiFi(const String& ssid, const String& password, const String& ssidNumber) {
  WiFi.begin(ssid.c_str(), password.c_str());
  Serial.print("Connecting to WiFi SSID ");
  Serial.print(ssidNumber); Serial.print(": ");
  Serial.println(ssid);

  unsigned long startAttemptTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 10000) {
    Serial.print(".");
    delay(1000);
  }

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("");
    Serial.println("Failed to connect to WiFi SSID " + ssidNumber);
    startAPMode();
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("");
    Serial.println("Connected to WiFi SSID " + ssidNumber);
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
  }
}

void ESPWiFiManager::startAPMode() {
  WiFi.softAP("ESP-01", "accessmyesp");
  Serial.print("AP IP address: ");
  Serial.println(WiFi.softAPIP());
}

void ESPWiFiManager::handleRoot() {
  String html = "<!DOCTYPE html><html lang='en'><head><meta charset='UTF-8'>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
  html += "<title>WiFi Scan</title>";
  html += "<style>@import url('https://fonts.googleapis.com/css2?family=Roboto:wght@400;700&display=swap');";
  html += "body { font-family: 'Roboto', sans-serif; font-size: 16px; text-align: left; padding: 2em; background-color: #121212; color: #ffffff; }";
  html += "input[type=text], select { width: 80%; padding: 0.5em; margin-top: 0.5em; border: 1px solid #444; border-radius: 5px; background-color: #1e1e1e; color: #ffffff; }";
  html += "input[type=submit] { padding: 0.5em 1em; margin-top: 0.5em; border: none; border-radius: 5px; background-color: #2980b9; color: white; cursor: pointer; }";
  html += "select { width: 80%; padding: 0.5em; margin-top: 0.5em; border: 1px solid #444; border-radius: 5px; background-color: #1e1e1e; color: #ffffff; }</style></head><body>";
  html += "<h1>Select WiFi Networks</h1><form method='POST'><ul>";

  int n = WiFi.scanNetworks();

  html += "<li>Primary Network:<br>";
  html += "<select name='primary_ssid'>";
  html += "<option value=''>Select Primary WiFi Network</option>";

  for (int i = 0; i < n; ++i) {
    int signalPercent = map(WiFi.RSSI(i), -100, -50, 0, 100); // Calculate signal percentage
    signalPercent = min(signalPercent, 100); // Ensure signal percentage does not exceed 100%

    html += "<option value='" + WiFi.SSID(i) + "'>" + WiFi.SSID(i) + " (" + String(signalPercent) + "%)</option>";
  }

  html += "</select>";
  html += "<input type='text' name='primary_password' placeholder='Enter Primary WiFi Password'></li>";

  html += "<li>Secondary Network:<br>";
  html += "<select name='secondary_ssid'>";
  html += "<option value=''>Select Secondary WiFi Network</option>";

  for (int i = 0; i < n; ++i) {
    int signalPercent = map(WiFi.RSSI(i), -100, -50, 0, 100); // Calculate signal percentage
    signalPercent = min(signalPercent, 100); // Ensure signal percentage does not exceed 100%

    html += "<option value='" + WiFi.SSID(i) + "'>" + WiFi.SSID(i) + " (" + String(signalPercent) + "%)</option>";
  }

  html += "</select>";
  html += "<input type='text' name='secondary_password' placeholder='Enter Secondary WiFi Password'></li>";

  html += "</ul>";
  html += "<input type='submit' value='Save'></form>";
  html += "<br>Enter network credentials";
  html += "</body></html>";
  this->server.send(200, "text/html", html);
}

void ESPWiFiManager::handleRootPost() {
  if (server.hasArg("primary_ssid") && server.hasArg("primary_password")) {
    storedStrings[0] = server.arg("primary_ssid");
    storedStrings[1] = server.arg("primary_password");
    saveStringToEEPROM(0, storedStrings[0]);
    saveStringToEEPROM(1, storedStrings[1]);
  }
  if (server.hasArg("secondary_ssid") && server.hasArg("secondary_password")) {
    storedStrings[2] = server.arg("secondary_ssid");
    storedStrings[3] = server.arg("secondary_password");
    saveStringToEEPROM(2, storedStrings[2]);
    saveStringToEEPROM(3, storedStrings[3]);
  }

  String html = "<!DOCTYPE html><html lang='en'><head><meta charset='UTF-8'>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
  html += "<title>Rebooting...</title>";
  html += "<style>@import url('https://fonts.googleapis.com/css2?family=Roboto:wght@400;700&display=swap');";
  html += "body { font-family: 'Roboto', sans-serif; font-size: 16px; text-align: center; padding: 2em; }";
  html += "</style></head><body>";
  html += "<h1>Credentials Saved</h1>";
  html += "<p>Rebooting in <span id='countdown'>5</span> seconds...</p>";
  html += "<script>";
  html += "var countdown = 5; ";
  html += "var countdownInterval = setInterval(function() { ";
  html += "countdown--; ";
  html += "document.getElementById('countdown').textContent = countdown; ";
  html += "if (countdown <= 0) { ";
  html += "clearInterval(countdownInterval); ";
  html += "window.location = '/reboot'; ";
  html += "} ";
  html += "}, 1000);";
  html += "</script>";
  html += "</body></html>";

  server.send(200, "text/html", html);
  delay(5000);
  ESP.restart();
}
