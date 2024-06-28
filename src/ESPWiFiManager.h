#ifndef ESPWiFiManager_h
#define ESPWiFiManager_h

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>

class ESPWiFiManager {
public:
  ESPWiFiManager();
  void begin();
  void handleClient();

private:
  ESP8266WebServer server;
  const int maxStrings = 4;
  const int maxLength = 100;
  const int eepromSize = maxStrings * maxLength;
  String storedStrings[4];

  void loadStringsFromEEPROM();
  void saveStringToEEPROM(int index, const String& str);
  void connectToWiFi(const String& ssid, const String& password, const String& ssidNumber);
  void startAPMode();
  void handleRoot();
  void handleRootPost();
};

#endif
