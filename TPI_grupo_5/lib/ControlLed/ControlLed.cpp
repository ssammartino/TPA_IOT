#include "ControlLed.h"

ControlLed::ControlLed(short pin) : _pin(pin) {
  pinMode(_pin, OUTPUT);
}

void ControlLed::encender() { digitalWrite(_pin, HIGH); }
void ControlLed::apagar()   { digitalWrite(_pin, LOW); }

bool ControlLed::estaEncendido() const {
  return digitalRead(_pin) == HIGH;
}

String ControlLed::estado() const {
  return estaEncendido() ? "encendido" : "apagado";
}