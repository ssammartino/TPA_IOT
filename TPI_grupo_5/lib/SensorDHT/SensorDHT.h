#ifndef SENSOR_DHT_H
#define SENSOR_DHT_H

#include <Arduino.h>
#include <DHT.h>

// Envoltorio del sensor DHT22 (temperatura y humedad), como en la Práctica 7.
// Si la lectura falla, temperatura() y humedad() devuelven NaN: hay que
// comprobarlo con isnan() antes de usar el valor.
class SensorDHT {
  public:
    SensorDHT(short pin);
    void iniciar();
    float temperatura();   // En grados Celsius
    float humedad();       // Humedad relativa en %

  private:
    DHT sensor;
};

#endif
