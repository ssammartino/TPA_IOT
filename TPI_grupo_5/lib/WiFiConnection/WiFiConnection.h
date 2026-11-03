#ifndef WIFI_CONNECTION_H
#define WIFI_CONNECTION_H

#include <WiFi.h>

// Conexión del ESP32 a una red WiFi (Prácticas 4 a 7).
// A diferencia de las prácticas, connect() no espera para siempre: si no se
// conecta en timeoutMs devuelve false, y main.cpp lo reintenta más tarde.
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
