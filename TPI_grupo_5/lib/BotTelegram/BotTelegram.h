#ifndef BOT_TELEGRAM_H
#define BOT_TELEGRAM_H

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include "SensorDHT.h"

// Estas funciones siguen definidas en main.cpp, como en el archivo original.
// Las declaramos aquí para poder llamarlas desde BotTelegram.cpp.
float leerVoltajePot();
bool enviarThingSpeak(float temperatura, float humedad);
void registrarComando(const String &cmd);
void mostrarEnDisplay(const String &l1, const String &l2);
String estadoLed(short pin);

class BotTelegram {
  public:
    BotTelegram(const char *token, SensorDHT &sensor, float &umbralActual,
                bool &automatico, short pinVerde, short pinAzul);
    void iniciar();
    void actualizar();

  private:
    WiFiClientSecure client;
    UniversalTelegramBot bot;
    SensorDHT &dht;
    float &umbral;
    bool &modoAuto;
    short LED_VERDE;
    short LED_AZUL;
    unsigned long ant = 0;
    const short INTERVAL = 1000;
    const float UMBRAL_MIN = 1.0;
    const float UMBRAL_MAX = 1.5;
    const float UMBRAL_DEFECTO = 0.5;

    void procesarMensajes(int cantidad);
    void procesarComando(const String &chatId, const String &texto);
    void cmdStart(const String &chatId);
    void cmdUmbral(const String &chatId, const String &texto);
    void cmdEstado(const String &chatId);
    void cmdAuto(const String &chatId, bool activar);
    void cmdThingUp(const String &chatId);
    void cmdDisplay(const String &chatId, const String &sub);
    void cmdLed(const String &chatId, short pin, bool encender);
};

#endif
