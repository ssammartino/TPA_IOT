#ifndef CONTROL_LED_H
#define CONTROL_LED_H

#include <Arduino.h>

// Maneja un LED conectado a un pin digital (pinMode + digitalWrite, como en la Práctica 1).
class ControlLed {
  public:
    ControlLed(short pin);
    void iniciar();
    void encender();
    void apagar();
    bool estaEncendido() const;
    String estado() const;   // "encendido" / "apagado", listo para mostrar

  private:
    short _pin;
};

#endif
