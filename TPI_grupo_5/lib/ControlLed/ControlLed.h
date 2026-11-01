#ifndef LED_CONTROL_H
#define LED_CONTROL_H

#include <Arduino.h>

class ControlLed {
  public:
    ControlLed(short pin);
    void encender();
    void apagar();
    bool estaEncendido() const;
    String estado() const;   // "encendido" / "apagado", listo para mostrar

  private:
    short _pin;
};

#endif