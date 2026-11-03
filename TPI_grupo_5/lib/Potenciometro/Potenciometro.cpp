#include "Potenciometro.h"

Potenciometro::Potenciometro(short pin) : _pin(pin) {}

// Con 12 bits, analogRead devuelve valores de 0 a 4095.
void Potenciometro::iniciar() {
  pinMode(_pin, INPUT);
  analogReadResolution(12);
}

float Potenciometro::leerVoltaje() {
  // Conversion a 0-3.3 V usada en Clase02.
  // Regla de tres: 4095 equivale a 3.3 V.
  return analogRead(_pin) * 3.3 / 4095.0;
}
