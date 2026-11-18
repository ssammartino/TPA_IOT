#ifndef CONTROL_LED_H
#define CONTROL_LED_H

#include <Arduino.h>

class ControlLed {
  public:
    ControlLed(short pin);
    void iniciar();
    void encender();
    void apagar();
    bool estaEncendido() const;
    String estado() const;   // "encendido" / "apagado"

  private:
    short _pin;
};

#endif
