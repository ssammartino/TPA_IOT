#ifndef CLIENTE_THINGSPEAK_H
#define CLIENTE_THINGSPEAK_H

#include <Arduino.h>
#include <WiFi.h>
#include <ThingSpeak.h>

class ClienteThingSpeak {
  public:
    ClienteThingSpeak(unsigned long canal, const char *apiKey,
                     unsigned long minEntreEnvios = 15000);
    void iniciar();
    bool enviar(float temperatura, float humedad);
    bool disponible() const;
    unsigned long esperaRestante() const;
    int ultimoCodigo() const;

  private:
    WiFiClient _client;
    unsigned long _canal;
    const char *_apiKey;
    unsigned long _minEntreEnvios;
    unsigned long _ultimoIntento = 0;
    bool _huboIntento = false;
    bool _iniciado = false;
    int _ultimoCodigo = 0;
};

#endif
