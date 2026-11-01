

#include <Arduino.h>
#include <WiFi.h>
#include <UniversalTelegramBot.h>
#include <WifiClientSecure.h>

// put function declarations here:
const short LED_VERDE = 23;
const short LED_AZUL = 2;

const char *SSID = "Wokwi-GUEST"; // Red WiFi
const char *PASS = ""; 
const char *API_KEY_TELEGRAM = "8931640257:AAHYxnhJPxTUbLoU6N5WyVL5yoZpiE8moMI";


WiFiClientSecure client;
UniversalTelegramBot bot(API_KEY_TELEGRAM, client);
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(LED_VERDE, OUTPUT);
  pinMode(LED_AZUL, OUTPUT);
  

  
  WiFi.begin(SSID, PASS);
  // Bucle bloqueante hasta establecer conexión WiFi
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("");
  Serial.println("WiFi conectado");
  Serial.println(WiFi.localIP()); // Muestra la IP asignada por el DHCP



  
}

void loop() {
  // put your main code here, to run repeatedly:
}

// put function definitions here:
int myFunction(int x, int y) {
  
}

//transformarValores (de 2^nosecuanto a °C, y de 2^ns a %)


