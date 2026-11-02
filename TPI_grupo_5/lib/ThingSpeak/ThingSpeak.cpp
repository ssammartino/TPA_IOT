#include "ClienteThingSpeak.h"

ClienteThingSpeak::ClienteThingSpeak(unsigned long canal, const char *apiKey,
                                     unsigned long minEntreEnvios)
    : _canal(canal), _apiKey(apiKey), _minEntreEnvios(minEntreEnvios) {}

void ClienteThingSpeak::iniciar() {
  ThingSpeak.begin(_client);
}

bool ClienteThingSpeak::enviar(float temperatura, float humedad) {
  if (_huboEnvio && millis() - _ultimoEnvio < _minEntreEnvios) return false;

  ThingSpeak.setField(1, temperatura);
  ThingSpeak.setField(2, humedad);
  int codigo = ThingSpeak.writeFields(_canal, _apiKey);

  _ultimoEnvio = millis();
  _huboEnvio = true;
  return codigo == 200;
}