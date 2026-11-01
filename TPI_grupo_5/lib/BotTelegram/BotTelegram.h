#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>

// Conexion y prueba de eco de Practica06 / ejemplo ESP32 EchoBot del ZIP.
class BotTelegram {
public:
  explicit BotTelegram(const char *token);
  bool iniciar();
  void actualizar();

private:
  WiFiClientSecure client;
  UniversalTelegramBot bot;
  unsigned long ant = 0;
  bool primeraConexion = true;
  static const unsigned long INTERVAL = 1000;

  void procesarMensajes(int cantidad);
};
