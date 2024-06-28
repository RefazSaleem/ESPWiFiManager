#include "ESPWiFiManager.h"

ESPWiFiManager wifiManager;

void setup() {
  wifiManager.begin();
}

void loop() {
  wifiManager.handleClient();
}
