#ifndef SENSOR_DHT_H
#define SENSOR_DHT_H

#include<Arduino.h>
#include<DHT.h>

 class SensorDHT{
    public:
        SensorDHT(short pin);
        void iniciar();
        float temperatura();
        float humedad();
    private:
        DHT sensor;
 };

#endif
