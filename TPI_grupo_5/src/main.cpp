#include <Arduino.h>
#include "SensorDHT.h"
#include "WiFiConection.h"
#include "BotTelegram.h"
#include "Potenciometro.h"
#include "ControlLed.h"
#include "Display.h"
#include "ClienteThingSpeak.h"

const short PIN_DHT = 33;
const short PIN_POT = 32;
const short LED_VERDE = 23;
const short LED_AZUL = 2;
const short OLED_SDA = 21;
const short OLED_SCL = 22;
const unsigned long INTERVAL_SENSORES = 2000;
const unsigned long INTERVAL_RECONEXION = 15000;
const unsigned long INTERVAL_THINGSPEAK = 20000;

const char *SSID = "Wokwi-GUEST";
const char *PASS = "";
const char *API_KEY_TELEGRAM = "8931640257:AAHYxnhJPxTUbLoU6N5WyVL5yoZpiE8moMI";
const char *API_KEY_TS = "6O8Z700NWK3XDHMY";
const unsigned long CHANNEL_TS = 3561793;

SensorDHT sensor(PIN_DHT);
WifiConnection wifi(SSID, PASS);
Potenciometro pot(PIN_POT);
ControlLed ledVerde(LED_VERDE);
ControlLed ledAzul(LED_AZUL);
Display display;
ClienteThingSpeak nube(CHANNEL_TS, API_KEY_TS);
float umbral = 0.5;
bool modoAuto = false;
BotTelegram telegram(API_KEY_TELEGRAM, sensor, pot, ledVerde, ledAzul,
                     display, nube, umbral, modoAuto);

bool telegramConectado = false;
unsigned long ultimaLectura = 0;
unsigned long ultimoIntento = 0;
unsigned long ultimaSubida = 0;

void inicializarPerifericos();
bool conectarWiFi();
void comprobarSensores();
void actualizarControlAutomatico();
void enviarTelemetriaPeriodica();

void setup() {
  Serial.begin(9600);
  inicializarPerifericos();
  nube.iniciar();
  if (conectarWiFi()) {
    telegramConectado = telegram.iniciar();
  }
  ultimoIntento = millis();
  ultimaSubida = millis();
}

void loop() {
  actualizarControlAutomatico();
  comprobarSensores();

  if (!wifi.connected()) {
    telegramConectado = false;
  }
  if (!telegramConectado && millis() - ultimoIntento >= INTERVAL_RECONEXION) {
    ultimoIntento = millis();
    if (wifi.connected() || conectarWiFi()) {
      telegramConectado = telegram.iniciar();
    }
  }
  if (telegramConectado) {
    telegram.actualizar();
  }
  actualizarControlAutomatico();
  enviarTelemetriaPeriodica();
}

void inicializarPerifericos() {
  ledVerde.iniciar();
  ledAzul.iniciar();
  pot.iniciar();
  sensor.iniciar();
  if (display.iniciar(OLED_SDA, OLED_SCL)) {
    display.mostrar("TPI Grupo 5", "ESP32 iniciado");
    Serial.println("OLED inicializado en 0x3c.");
  } else {
    Serial.println("No se pudo inicializar el OLED.");
  }
}

bool conectarWiFi() {
  Serial.println("Conectando a WiFi...");
  if (!wifi.connect()) {
    Serial.println("No se pudo conectar a WiFi. Se reintentara.");
    return false;
  }
  Serial.println("WiFi conectado. IP:");
  Serial.println(wifi.localIP());
  return true;
}

void comprobarSensores() {
  if (millis() - ultimaLectura < INTERVAL_SENSORES) {
    return;
  }
  ultimaLectura = millis();
  const float temperatura = sensor.temperatura();
  const float humedad = sensor.humedad();
  if (isnan(temperatura) || isnan(humedad)) {
    Serial.println("Error de lectura del DHT22.");
  } else {
    Serial.printf("DHT22 | Temperatura: %.1f C | Humedad: %.1f %%\r\n", temperatura, humedad);
  }
  Serial.printf("Potenciometro GPIO 32: %.2f V\r\n", pot.leerVoltaje());
}

void actualizarControlAutomatico() {
  if (modoAuto) {
    if (pot.leerVoltaje() > umbral) {
      ledAzul.encender();
    } else {
      ledAzul.apagar();
    }
  }
}

void enviarTelemetriaPeriodica() {
  if (!wifi.connected() || millis() - ultimaSubida < INTERVAL_THINGSPEAK || !nube.disponible()) {
    return;
  }
  ultimaSubida = millis();
  const float temperatura = sensor.temperatura();
  const float humedad = sensor.humedad();
  if (isnan(temperatura) || isnan(humedad)) {
    Serial.println("ThingSpeak: lectura DHT22 invalida, se omite el envio.");
    return;
  }
  if (nube.enviar(temperatura, humedad)) {
    Serial.println("ThingSpeak: temperatura y humedad enviadas (HTTP 200).");
  } else {
    Serial.printf("ThingSpeak: fallo de envio, codigo %d.\r\n", nube.ultimoCodigo());
  }
}
