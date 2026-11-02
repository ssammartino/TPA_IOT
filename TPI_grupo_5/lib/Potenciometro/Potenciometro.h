#ifndef POTENCIOMETRO_H
#define POTENCIOMETRO_H

#include <Arduino.h>

class Potenciometro {
  public:
    explicit Potenciometro(short pin);
    void iniciar();
    float leerVoltaje();

  private:
    short _pin;
};

#endif
