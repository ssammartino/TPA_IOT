// TPI Grupo 5 - ESP32
// Cada periférico tiene su clase en /lib. Acá se crean los objetos y se
// coordinan en el loop. Usamos millis() en vez de delay() para no bloquear.

#include <Arduino.h>
#include "SensorDHT.h"
#include "WiFiConnection.h"
#include "BotTelegram.h"
#include "Potenciometro.h"
#include "ControlLed.h"
#include "Display.h"
#include "ClienteThingSpeak.h"
#include "secrets.h"   // credenciales, ver secrets.example.h

// Pines
const short PIN_DHT = 33;
const short PIN_POT = 32;
const short LED_VERDE = 23;
const short LED_AZUL = 2;
// OLED por I2C
const short OLED_SDA = 21;
const short OLED_SCL = 22;

// Intervalos en ms (ThingSpeak gratis pide mínimo 15 s)
const unsigned long INTERVAL_SENSORES = 2000;
const unsigned long INTERVAL_RECONEXION = 15000;
const unsigned long INTERVAL_THINGSPEAK = 20000;

// Credenciales
const char *SSID = SECRET_SSID;
const char *PASS = SECRET_PASS;
const char *API_KEY_TELEGRAM = SECRET_TELEGRAM_TOKEN;
const char *API_KEY_TS = SECRET_TS_API_KEY;
const unsigned long CHANNEL_TS = SECRET_TS_CHANNEL;

// Periféricos
SensorDHT sensor(PIN_DHT);
WifiConnection wifi(SSID, PASS);
Potenciometro pot(PIN_POT);
ControlLed ledVerde(LED_VERDE);
ControlLed ledAzul(LED_AZUL);
Display display;
ClienteThingSpeak nube(CHANNEL_TS, API_KEY_TS);
// Se pasan por referencia al bot, así los cambios desde Telegram
// se ven acá directamente.
float umbral = 0.5;
bool modoAuto = false;
BotTelegram telegram(API_KEY_TELEGRAM, sensor, pot, ledVerde, ledAzul,
                     display, nube, umbral, modoAuto);

// Última vez que se hizo cada tarea (millis)
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
  // Telegram solo si hay WiFi
  if (conectarWiFi()) {
    telegramConectado = telegram.iniciar();
    display.mostrar("WiFi conectado", telegramConectado ? "Telegram listo" : "Telegram pendiente");
  }
  ultimoIntento = millis();
  ultimaSubida = millis();
}

void loop() {
  comprobarSensores();

  // Sin WiFi tampoco hay Telegram
  if (!wifi.connected()) {
    telegramConectado = false;
  }
  // Reintenta la conexión cada 15 s
  if (!telegramConectado && millis() - ultimoIntento >= INTERVAL_RECONEXION) {
    ultimoIntento = millis();
    if (wifi.connected() || conectarWiFi()) {
      telegramConectado = telegram.iniciar();
      display.mostrar("WiFi conectado", telegramConectado ? "Telegram listo" : "Telegram pendiente");
    }
  }
  if (telegramConectado) {
    telegram.actualizar();
  }
  // Después de Telegram, para aplicar enseguida los cambios de modo/umbral
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
  display.mostrar("Conectando a WiFi...", "");
  Serial.println("Conectando a WiFi...");
  if (!wifi.connect()) {
    Serial.println("No se pudo conectar a WiFi. Se reintentara.");
    display.mostrar("Conexion fallida", "a WiFi");
    return false;
  }
  Serial.println("WiFi conectado. IP:");
  Serial.println(wifi.localIP());
  display.mostrar("Conexion exitosa", "a WiFi");
  return true;
}

// Imprime las lecturas por Serial cada 2 s
void comprobarSensores() {
  if (millis() - ultimaLectura < INTERVAL_SENSORES) {
    return;
  }
  ultimaLectura = millis();
  const float temperatura = sensor.temperatura();
  const float humedad = sensor.humedad();
  // NaN = falló la lectura
  if (isnan(temperatura) || isnan(humedad)) {
    Serial.println("Error de lectura del DHT22.");
  } else {
    Serial.printf("DHT22 | Temperatura: %.1f C | Humedad: %.1f %%\r\n", temperatura, humedad);
  }
  Serial.printf("Potenciometro GPIO 32: %.2f V\r\n", pot.leerVoltaje());
}

// Modo auto: LED azul prendido si el pot supera el umbral
void actualizarControlAutomatico() {
  if (modoAuto) {
    if (pot.leerVoltaje() > umbral) {
      ledAzul.encender();
    } else {
      ledAzul.apagar();
    }
  }
}

// Sube temperatura y humedad a ThingSpeak cada 20 s
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
