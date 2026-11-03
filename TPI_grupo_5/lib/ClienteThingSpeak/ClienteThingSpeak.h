#ifndef CLIENTE_THINGSPEAK_H
#define CLIENTE_THINGSPEAK_H

#include <Arduino.h>
#include <WiFi.h>
#include <ThingSpeak.h>

// Envía temperatura y humedad a un canal de ThingSpeak (basado en la Práctica 7).
// Además controla que pasen al menos 15 s entre envíos, que es el límite
// de la cuenta gratuita de ThingSpeak.
class ClienteThingSpeak {
  public:
    ClienteThingSpeak(unsigned long canal, const char *apiKey,
                     unsigned long minEntreEnvios = 15000);
    void iniciar();
    // Devuelve true si ThingSpeak respondió 200 (envío correcto).
    bool enviar(float temperatura, float humedad);
    // true si ya pasó el tiempo mínimo desde el último envío.
    bool disponible() const;
    // Milisegundos que faltan para poder volver a enviar.
    unsigned long esperaRestante() const;
    // Código de resultado del último envío (200 = OK; negativos = error).
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
