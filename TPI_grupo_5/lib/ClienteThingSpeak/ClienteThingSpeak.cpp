#include "ClienteThingSpeak.h"

// Si se pide un intervalo menor a 15 s, se fuerza a 15 s (límite de ThingSpeak).
ClienteThingSpeak::ClienteThingSpeak(unsigned long canal, const char *apiKey,
                                   unsigned long minEntreEnvios)
    : _canal(canal), _apiKey(apiKey),
      _minEntreEnvios(minEntreEnvios < 15000 ? 15000 : minEntreEnvios) {}

// Igual que ThingSpeak.begin(client) en la Práctica 7.
void ClienteThingSpeak::iniciar() {
  _iniciado = ThingSpeak.begin(_client);
}

// Si nunca se envió nada, se puede enviar ya (espera 0).
unsigned long ClienteThingSpeak::esperaRestante() const {
  if (!_huboIntento) return 0;
  const unsigned long transcurrido = millis() - _ultimoIntento;
  return transcurrido < _minEntreEnvios ? _minEntreEnvios - transcurrido : 0;
}

bool ClienteThingSpeak::disponible() const {
  return esperaRestante() == 0;
}

int ClienteThingSpeak::ultimoCodigo() const {
  return _ultimoCodigo;
}

bool ClienteThingSpeak::enviar(float temperatura, float humedad) {
  // Antes de enviar se valida todo; si algo falla se guarda un código de error
  // de la librería ThingSpeak para poder informarlo.
  if (isnan(temperatura) || isnan(humedad)) {
    _ultimoCodigo = TS_ERR_OUT_OF_RANGE;
    return false;
  }
  if (!_iniciado) {
    _ultimoCodigo = TS_ERR_UNEXPECTED_FAIL;
    return false;
  }
  if (WiFi.status() != WL_CONNECTED) {
    _ultimoCodigo = TS_ERR_CONNECT_FAILED;
    return false;
  }
  if (!disponible()) {
    _ultimoCodigo = TS_ERR_NOT_INSERTED;
    return false;
  }

  // Practica de ThingSpeak: temperatura en campo 1, humedad en campo 2.
  ThingSpeak.setField(1, temperatura);
  ThingSpeak.setField(2, humedad);
  _ultimoCodigo = ThingSpeak.writeFields(_canal, _apiKey);
  // Espaciar tambien intentos HTTP fallidos para no saturar las peticiones.
  _ultimoIntento = millis();
  _huboIntento = true;
  return _ultimoCodigo == TS_OK_SUCCESS;
}
