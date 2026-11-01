#include "WiFiConection.h"

WifiConnection::WifiConnection(const char* ssid, const char* password)
    : _ssid(ssid), _password(password) {}

bool WifiConnection::connect(uint32_t timeoutMs) {
  WiFi.mode(WIFI_STA);
  WiFi.begin(_ssid, _password);

  const uint32_t startedAt = millis();
  while (WiFi.status() != WL_CONNECTED) {
    if (millis() - startedAt >= timeoutMs) {
      WiFi.disconnect();
      return false;
    }
    delay(100);
  }

  return true;
}

bool WifiConnection::connected() const {
  return WiFi.status() == WL_CONNECTED;
}

IPAddress WifiConnection::localIP() const {
  return WiFi.localIP();
}