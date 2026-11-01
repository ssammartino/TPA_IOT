#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include "SensorDHT.h"
#include "WiFiConection.h"
#include "BotTelegram.h"

const short PIN_DHT = 33;
const short PIN_POT = 32;
const short LED_VERDE = 23;
const short LED_AZUL = 2;
const short OLED_SDA = 21;
const short OLED_SCL = 22;
const unsigned long INTERVAL_SENSORES = 2000;
const unsigned long INTERVAL_RECONEXION = 15000;

const char *SSID = "Wokwi-GUEST";
const char *PASS = "";
const char *API_KEY_TELEGRAM = "8931640257:AAHYxnhJPxTUbLoU6N5WyVL5yoZpiE8moMI";

SensorDHT sensor(PIN_DHT);
WifiConnection wifi(SSID, PASS);
BotTelegram telegram(API_KEY_TELEGRAM);
// Mismo controlador y resolucion que el ejemplo OLED de Clase03.
Adafruit_SH1106G display(128, 64, &Wire, -1);

bool telegramConectado = false;
unsigned long ultimaLectura = 0;
unsigned long ultimoIntento = 0;

float leerVoltajePot();
void inicializarPerifericos();
bool conectarWiFi();
void comprobarSensores();

void setup() {
  Serial.begin(9600);
  inicializarPerifericos();

  if (conectarWiFi()) {
    telegramConectado = telegram.iniciar();
  }
  ultimoIntento = millis();
}

void loop() {
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
}

void inicializarPerifericos() {
  pinMode(LED_VERDE, OUTPUT);
  pinMode(LED_AZUL, OUTPUT);
  digitalWrite(LED_VERDE, LOW);
  digitalWrite(LED_AZUL, LOW);

  pinMode(PIN_POT, INPUT);
  analogReadResolution(12);
  sensor.iniciar();

  Wire.begin(OLED_SDA, OLED_SCL);
  if (display.begin(0x3c, true)) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SH110X_WHITE);
    display.setCursor(0, 0);
    display.println("TPI Grupo 5");
    display.println("ESP32 iniciado");
    display.display();
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

float leerVoltajePot() {
  // Conversion de 12 bits a voltaje usada en Clase02.
  const int lecturaADC = analogRead(PIN_POT);
  return (lecturaADC * 3.3) / 4095;
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
  Serial.printf("Potenciometro GPIO 32: %.2f V\r\n", leerVoltajePot());
}
