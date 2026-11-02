#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include "SensorDHT.h"
#include "Potenciometro.h"
#include "ControlLed.h"
#include "Display.h"
#include "ClienteThingSpeak.h"

class BotTelegram {
public:
  BotTelegram(const char *token, SensorDHT &sensor, Potenciometro &pot,
              ControlLed &ledVerde, ControlLed &ledAzul, Display &display,
              ClienteThingSpeak &thingspeak, float &umbralActual, bool &automatico);
  bool iniciar();
  void actualizar();

private:
  WiFiClientSecure client;
  UniversalTelegramBot bot;
  SensorDHT &dht;
  Potenciometro &pot;
  ControlLed &ledVerde;
  ControlLed &ledAzul;
  Display &display;
  ClienteThingSpeak &thingspeak;
  float &umbral;
  bool &modoAuto;
  unsigned long ant = 0;
  bool primeraConexion = true;
  static const unsigned long INTERVAL = 1000;
  const float UMBRAL_MIN = 1.0;
  const float UMBRAL_MAX = 1.5;
  const float UMBRAL_DEFECTO = 0.5;

  void procesarMensajes(int cantidad);
  void procesarComando(const String &chatId, const String &texto);
  void cmdStart(const String &chatId);
  bool cmdUmbral(const String &chatId, const String &texto);
  bool cmdEstado(const String &chatId);
  void cmdAuto(const String &chatId, bool activar);
  bool cmdThingUp(const String &chatId);
  bool cmdDisplay(const String &chatId, const String &sub);
  bool cmdLed(const String &chatId, ControlLed &led, const String &nombre, bool encender);
};
