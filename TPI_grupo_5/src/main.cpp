#include <Arduino.h>
#include <UniversalTelegramBot.h>
#include <ThingSpeak.h>
#include "WiFiConection.h"

// put function declarations here:
const short LED_VERDE = 23;
const short LED_AZUL = 2;
const char CHANNEL_TS = 3561793
const char INTERVAL = 1000
const char *SSID = "Wokwi-GUEST"; // Red WiFi
const char *PASS = ""; 
const char *API_KEY_TELEGRAM = "8931640257:AAHYxnhJPxTUbLoU6N5WyVL5yoZpiE8moMI";
const char *API_KEY_TS = "6O8Z700NWK3XDHMY"
WifiConnection wifi(SSID, PASS);
WiFiClientSecure client;
UniversalTelegramBot bot(API_KEY_TELEGRAM, client);
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(LED_VERDE, OUTPUT);
  pinMode(LED_AZUL, OUTPUT);
  
  wifi.connect();
  // Bucle bloqueante hasta establecer conexión WiFi
  if (wifi.connect()) {
    Serial.println(wifi.localIP());
  } else {
    Serial.println("No se pudo conectar al WiFi");
  }
  

  
}

void loop() {

  // Parte para leer el sensor
  if(millis()-time_ant >= INTERVAL){
    float t = sensor.readTemperature();
    float h = sensor.readHumidity();

    if(isnan(t) || isnan(h)){
      Serial.print("Error en la lectura del sensor DHT");
      return;
    }
    ThingSpeak.setField(1, t)
    ThingSpeak.setField(2, h)
    Serial.printf("Enviando datos | Temp: %.2f °C y Hum: %.2f %", t, h);
    ThingSpeak.writeFields(CHANNEL_ID, API_KEY);
  }

  // Parte para leer msjs de telegram
  if (millis() - ant > INTERVAL){
    ant = millis()
    int n = bot.getUpdates(bot.last_message_received + 1); // Consulta a telegram cuantos msjs nuevos llegaron
    while(n){ //mientras haya mensajes
      procesarMensajes(n);
      n = bot.getUpdates(bot.last_message_received + 1); // vuelve a llamar para ver si se mandaron msjs nuevos mientras se ejecutaba procesarMensajes
    }


  }
}

// put function definitions here:
int myFunction(int x, int y) {
  return x + y;
}


void procesarMensajes(int n){
    for (size_t i = 0; i < n; i++)
    {
      String chat_id = bot.messages[i].chat_id;
      String text = bot.messages[i].text;
      Serial.println("Mensaje recibido: " + text );
      
      //codigo ESP32:
      if(text == "/start"){
        bot.sendMessage(chat_id, "Bienvenido al Bot ESP32\n");
        bot.sendMessage(chat_id, "COMANDO LedON enciende el LED\n");
        bot.sendMessage(chat_id, "COMANDO LedOFF apaga el LED");      
      } else if(text == "/thingup"){
        if(millis()-time_ant >= INTERVAL){
        float t = sensor.temperatura();
        float h = sensor.humedad();

        if(isnan(t) || isnan(h)){
          Serial.print("Error en la lectura del sensor DHT");
          return;
        }
        ThingSpeak.setField(1, t)
        ThingSpeak.setField(2, h)
        Serial.printf("Enviando datos | Temp: %.2f °C y Hum: %.2f %", t, h);
        ThingSpeak.writeFields(CHANNEL_ID, API_KEY);
      }
        bot.sendMessage(chat_id, "LED encendido!");
      } else if(text == "LedOFF"){
        digitalWrite(LED, LOW);
        bot.sendMessage(chat_id, "LED apagado!");
      } else{
        bot.sendMessage(chat_id, "Comando no reconocido");
      }      
    }
}

//transformarValores (de 2^nosecuanto a °C, y de 2^ns a %)
