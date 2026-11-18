#ifndef SENSOR_DHT_H
#define SENSOR_DHT_H

#include <Arduino.h>
#include <DHT.h>

// Si la lectura falla devuelve NaN, revisar con isnan()
class SensorDHT {
  public:
    SensorDHT(short pin);
    void iniciar();
    float temperatura();   // °C
    float humedad();       // %

  private:
    DHT sensor;
};

#endif
