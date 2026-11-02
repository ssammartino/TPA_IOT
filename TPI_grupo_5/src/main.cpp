#include <Arduino.h>
#include "WiFiConection.h"
#include "SensorDHT.h"
#include "Potenciometro.h"
#include "ControlLed.h"
#include "Display.h"
#include "ClienteThingSpeak.h"
#include "BotTelegram.h"

// ===== Pines =====
const short LED_VERDE = 23;
const short LED_AZUL = 2;
const short PIN_DHT = 33;
const short PIN_POT = 32;

// ===== ThingSpeak =====
const unsigned long CHANNEL_TS = 3561793;
const char *API_KEY_TS = "NUEVA_WRITE_KEY";   // revocá la anterior, quedó expuesta

// ===== WiFi =====
const char *SSID = "Wokwi-GUEST";
const char *PASS = "";

// ===== Telegram =====
const char *API_KEY_TELEGRAM = "TOKEN_NUEVO"; // revocá el anterior, quedó expuesto

// ===== Umbral / modo automático =====
float umbral = 0.5;
bool modoAuto = false;

// ===== Objetos =====
WifiConnection wifi(SSID, PASS);
SensorDHT sensor(PIN_DHT);
Potenciometro pot(PIN_POT);
ControlLed ledVerde(LED_VERDE);
ControlLed ledAzul(LED_AZUL);
Display display;
ClienteThingSpeak nube(CHANNEL_TS, API_KEY_TS);
BotTelegram bot(API_KEY_TELEGRAM, sensor, pot, ledVerde, ledAzul,
                display, nube, umbral, modoAuto);

void setup() {
  Serial.begin(9600);

  display.iniciar();
  display.mostrar("Conectando a WiFi...", "");

  if (wifi.connect()) {
    Serial.println(wifi.localIP());
    display.mostrar("Conexion exitosa", "a WiFi");
  } else {
    Serial.println("No se pudo conectar al WiFi");
    display.mostrar("Conexion fallida", "a WiFi");
    return;   // sin WiFi no tiene sentido seguir
  }

  // Hora necesaria para validar el certificado de Telegram (va ANTES de bot.iniciar)
  configTime(0, 0, "pool.ntp.org");
  time_t ahora = time(nullptr);
  while (ahora < 1700000000) {
    delay(200);
    ahora = time(nullptr);
  }

  nube.iniciar();
  bot.iniciar();
  display.mostrar("Listo", "");
}

void loop() {
  if (!wifi.connected()) return;

  bot.actualizar();

  if (modoAuto) {
    (pot.leerVoltaje() > umbral) ? ledAzul.encender() : ledAzul.apagar();
  }
}