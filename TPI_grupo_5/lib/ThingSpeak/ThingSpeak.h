#ifndef CLIENTE_THINGSPEAK_H
#define CLIENTE_THINGSPEAK_H

#include <Arduino.h>
#include <WiFi.h>
#include <ThingSpeak.h>

class ThingSpeak {
  public:
    ThingSpeak(unsigned long canal, const char *apiKey,
                       unsigned long minEntreEnvios = 15000);
    void iniciar();
    bool enviar(float temperatura, float humedad);

  private:
    WiFiClient _client;
    unsigned long _canal;
    const char *_apiKey;
    unsigned long _minEntreEnvios;
    unsigned long _ultimoEnvio = 0;
    bool _huboEnvio = false;
};

#endif