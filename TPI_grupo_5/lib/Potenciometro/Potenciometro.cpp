#include "Potenciometro.h"

Potenciometro::Potenciometro(short pin) : _pin(pin) {}

// 12 bits: 0 a 4095
void Potenciometro::iniciar() {
  pinMode(_pin, INPUT);
  analogReadResolution(12);
}

float Potenciometro::leerVoltaje() {
  // 4095 = 3.3 V
  return analogRead(_pin) * 3.3 / 4095.0;
}
