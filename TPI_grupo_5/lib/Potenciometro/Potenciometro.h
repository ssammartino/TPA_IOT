#ifndef POTENCIOMETRO_H
#define POTENCIOMETRO_H

#include <Arduino.h>

// Lee un potenciómetro por el ADC y devuelve su voltaje (Práctica 1).
class Potenciometro {
  public:
    // explicit evita que un número se convierta solo en un Potenciometro por error.
    explicit Potenciometro(short pin);
    void iniciar();
    float leerVoltaje();

  private:
    short _pin;
};

#endif
