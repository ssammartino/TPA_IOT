#include "SensorDHT.h"

SensorDHT::SensorDHT(short pin): sensor(pin, DHT22){
}

void SensorDHT::iniciar(){
    sensor.begin();
}

 float SensorDHT::temperatura(){
    return sensor.readTemperature();
 }

 float SensorDHT::humedad(){
    return sensor.readHumidity();
 }
