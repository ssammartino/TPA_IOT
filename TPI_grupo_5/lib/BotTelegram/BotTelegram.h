#ifndef BOT_TELEGRAM_H
#define BOT_TELEGRAM_H

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include "SensorDHT.h"
#include "Potenciometro.h"
#include "ControlLed.h"
#include "Display.h"
#include "ClienteThingSpeak.h"

// Bot de Telegram (basado en la Práctica 6): recibe comandos de texto y
// actúa sobre los periféricos. No es dueño de ellos: guarda referencias (&)
// a los objetos creados en main.cpp, así controla exactamente los mismos.
class BotTelegram {
public:
  BotTelegram(const char *token, SensorDHT &sensor, Potenciometro &pot,
              ControlLed &ledVerde, ControlLed &ledAzul, Display &display,
              ClienteThingSpeak &thingspeak, float &umbralActual, bool &automatico);
  // Conecta con Telegram. Devuelve false si no hay WiFi o falla la conexión.
  bool iniciar();
  // Se llama en cada loop(): cada INTERVAL ms consulta y procesa mensajes nuevos.
  void actualizar();

private:
  // Conexión segura (HTTPS), obligatoria para la API de Telegram.
  WiFiClientSecure client;
  UniversalTelegramBot bot;
  // Referencias a los periféricos y al estado compartido con main.cpp.
  SensorDHT &dht;
  Potenciometro &pot;
  ControlLed &ledVerde;
  ControlLed &ledAzul;
  Display &display;
  ClienteThingSpeak &thingspeak;
  float &umbral;
  bool &modoAuto;
  // ant: momento de la última consulta a Telegram (control con millis()).
  unsigned long ant = 0;
  bool primeraConexion = true;
  static const unsigned long INTERVAL = 1000;
  // Rango permitido para /umbral y valor al que se vuelve si el dato es inválido.
  const float UMBRAL_MIN = 1.0;
  const float UMBRAL_MAX = 1.5;
  const float UMBRAL_DEFECTO = 0.5;

  void procesarMensajes(int cantidad);
  void procesarComando(const String &chatId, const String &texto);
  // Un método por comando. Los que devuelven bool indican si el comando
  // se ejecutó bien (solo esos se registran en el historial del display).
  void cmdStart(const String &chatId);
  bool cmdUmbral(const String &chatId, const String &texto);
  bool cmdEstado(const String &chatId);
  void cmdAuto(const String &chatId, bool activar);
  bool cmdThingUp(const String &chatId);
  bool cmdDisplay(const String &chatId, const String &sub);
  bool cmdLed(const String &chatId, ControlLed &led, const String &nombre, bool encender);
};

#endif
