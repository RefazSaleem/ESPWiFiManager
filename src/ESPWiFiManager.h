#ifndef ESPWiFiManager_h
#define ESPWiFiManager_h

#include <EEPROM.h>
#if defined(ESP32)
#include <WiFi.h>
#include <WebServer.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#endif

class ESPWiFiManager {
public:
  ESPWiFiManager();
  void begin();
  void handleClient();

private:
  void loadStringsFromEEPROM();
  void saveStringToEEPROM(int index, const String& str);
  void connectToWiFi(const String& ssid, const String& password, const String& ssidNumber);
  void startAPMode();
  void handleRoot();
  void handleRootPost();

  #if defined(ESP32)
  WebServer server;
  #elif defined(ESP8266)
  ESP8266WebServer server;
  #endif

  static const int eepromSize = 512;
  static const int maxLength = 32;
  static const int maxStrings = 4;
  String storedStrings[maxStrings];
};

#endif
