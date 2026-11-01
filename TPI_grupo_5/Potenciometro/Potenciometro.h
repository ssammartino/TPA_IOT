#ifndef POTENCIOMETRO_H
#define POTENCIOMETRO_H

#include <Arduino.h>

class Potenciometro {
  public:
    Potenciometro(short pin);
    float leerVoltaje();

  private:
    short _pin;
};

#endif