#include "Potenciometro.h"

Potenciometro::Potenciometro(short pin) : _pin(pin) {
  pinMode(_pin, INPUT);
}

float Potenciometro::leerVoltaje() {
  return analogRead(_pin) * 3.3 / 4095.0;
}