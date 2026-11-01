#include <Arduino.h>
#include <ThingSpeak.h>
#include "WiFiConection.h"
#include "BotTelegram.h"
#include "SensorDHT.h"

// PINES
const short LED_VERDE = 23;
const short LED_AZUL = 2;
const short PIN_DHT = 33;
const short PIN_POT = 32;

// ThingSpeak
const unsigned long CHANNEL_TS = 3561793;
const char *API_KEY_TS = "6O8Z700NWK3XDHMY";

//WiFi
const char *SSID = "Wokwi-GUEST"; // Red WiFi
const char *PASS = ""; 

//Telegram
const char *API_KEY_TELEGRAM = "8931640257:AAHYxnhJPxTUbLoU6N5WyVL5yoZpiE8moMI";

//Umbral potenciometro
float umbral = 0.5;
bool modoAuto = false;

// ===== OLED y buffer de últimos 3 comandos =====
Adafruit_SSD1306 oled(128, 64, &Wire, -1);
String ultimos[3] = {"", "", ""};


//Iniciar objetos
WifiConnection wifi(SSID, PASS);
WiFiClientSecure client;
SensorDHT sensor(PIN_DHT);
BotTelegram bot(API_KEY_TELEGRAM, sensor, umbral, modoAuto, LED_VERDE, LED_AZUL);

void setup() {

  Serial.begin(9600); //Definir baudios

  //Modos de los pines
  pinMode(LED_VERDE, OUTPUT);
  pinMode(LED_AZUL, OUTPUT);
  
  //Iniciar oled
  oled.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  mostrarEnDisplay("Conectando a WiFi...");

  //Para conectar a WiFi
  wifi.connect();
  // Bucle bloqueante hasta establecer conexión WiFi
  if (wifi.connect()) {
    Serial.println(wifi.localIP());
    mostrarEnDisplay("Conexion Exitosa a WiFi :D");

  } else {
    Serial.println("No se pudo conectar al WiFi");
    mostrarEnDisplay("Conectando fallida a WiFi X");
    return;
  }

  ThingSpeak.begin(client);
  bot.iniciar();
  
   // Hora necesaria para validar el certificado de Telegram
  configTime(0, 0, "pool.ntp.org");
  time_t ahora = time(nullptr);
  while (ahora < 1700000000) {
    delay(200);
    ahora = time(nullptr);
  }

}

void loop() {

  // Parte para leer msjs de telegram
  if (!wifi.connected()) return;
  bot.actualizar();

  if (modoAuto) {
    digitalWrite(LED_AZUL, leerVoltajePot() > umbral ? HIGH : LOW);
  }

  }


