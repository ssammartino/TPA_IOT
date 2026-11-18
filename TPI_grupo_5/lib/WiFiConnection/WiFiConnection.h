#ifndef WIFI_CONNECTION_H
#define WIFI_CONNECTION_H

#include <WiFi.h>

// connect() se rinde después de timeoutMs en vez de quedarse colgado
class WifiConnection {
public:
  WifiConnection(const char* ssid, const char* password);
  bool connect(uint32_t timeoutMs = 15000);
  bool connected() const;
  IPAddress localIP() const;

private:
  const char* _ssid;
  const char* _password;
};

#endif
