#include "ControlLed.h"

ControlLed::ControlLed(short pin) : _pin(pin) {}

void ControlLed::iniciar() {
  pinMode(_pin, OUTPUT);
  apagar();
}

void ControlLed::encender() { digitalWrite(_pin, HIGH); }
void ControlLed::apagar()   { digitalWrite(_pin, LOW); }

// En el ESP32 se puede leer un pin de salida
bool ControlLed::estaEncendido() const {
  return digitalRead(_pin) == HIGH;
}

String ControlLed::estado() const {
  return estaEncendido() ? "encendido" : "apagado";
}
