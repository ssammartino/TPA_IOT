#include "SensorDHT.h"

// Crea el objeto DHT de la librería indicando el pin y el modelo (DHT22).
SensorDHT::SensorDHT(short pin) : sensor(pin, DHT22) {}

void SensorDHT::iniciar() {
  sensor.begin();
}

float SensorDHT::temperatura() {
  return sensor.readTemperature();
}

float SensorDHT::humedad() {
  return sensor.readHumidity();
}
