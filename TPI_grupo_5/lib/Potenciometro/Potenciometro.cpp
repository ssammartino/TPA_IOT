#include "Potenciometro.h"

Potenciometro::Potenciometro(short pin) : _pin(pin) {}

void Potenciometro::iniciar() {
  pinMode(_pin, INPUT);
  analogReadResolution(12);
}

float Potenciometro::leerVoltaje() {
  // Conversion a 0-3.3 V usada en Clase02.
  return analogRead(_pin) * 3.3 / 4095.0;
}
