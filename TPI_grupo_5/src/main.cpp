/*
 * TPI Grupo 5 - Programa principal (ESP32)
 * ----------------------------------------
 * Integra lo visto en las prácticas:
 *   - Potenciómetro leído por ADC y convertido a voltaje (Práctica 1)
 *   - Sensor DHT22 + pantalla OLED SH1106 por I2C     (Prácticas 2 y 3)
 *   - Conexión WiFi                                   (Prácticas 4 y 5)
 *   - Bot de Telegram para recibir comandos           (Práctica 6)
 *   - Envío de datos a ThingSpeak                     (Práctica 7)
 *
 * Cada periférico está encapsulado en una clase dentro de /lib, y acá solo
 * se crean los objetos y se coordina qué hace cada uno en el loop().
 * Todas las tareas periódicas usan millis() en lugar de delay() para que
 * el programa nunca quede bloqueado y pueda atender todo "a la vez".
 */

#include <Arduino.h>
#include "SensorDHT.h"
#include "WiFiConnection.h"
#include "BotTelegram.h"
#include "Potenciometro.h"
#include "ControlLed.h"
#include "Display.h"
#include "ClienteThingSpeak.h"
#include "secrets.h"   // Credenciales (no se sube a GitHub; ver include/secrets.example.h)

// ---------------- Pines de conexión ----------------
const short PIN_DHT = 33;
const short PIN_POT = 32;
const short LED_VERDE = 23;
const short LED_AZUL = 2;
// I2C de la pantalla OLED: SDA = datos, SCL = reloj.
const short OLED_SDA = 21;
const short OLED_SCL = 22;

// ---------------- Intervalos (en milisegundos) ----------------
// Sensores: cada cuánto se imprimen las lecturas por Serial.
// Reconexión: cada cuánto se reintenta WiFi/Telegram si se cayó.
// ThingSpeak: cada cuánto se suben datos (la cuenta gratuita exige mínimo 15 s).
const unsigned long INTERVAL_SENSORES = 2000;
const unsigned long INTERVAL_RECONEXION = 15000;
const unsigned long INTERVAL_THINGSPEAK = 20000;

// ---------------- Credenciales ----------------
// Los valores reales están en include/secrets.h: red WiFi, token del bot
// (dado por BotFather), Write API Key y número de canal de ThingSpeak.
const char *SSID = SECRET_SSID;
const char *PASS = SECRET_PASS;
const char *API_KEY_TELEGRAM = SECRET_TELEGRAM_TOKEN;
const char *API_KEY_TS = SECRET_TS_API_KEY;
const unsigned long CHANNEL_TS = SECRET_TS_CHANNEL;

// ---------------- Objetos de cada periférico ----------------
SensorDHT sensor(PIN_DHT);
WifiConnection wifi(SSID, PASS);
Potenciometro pot(PIN_POT);
ControlLed ledVerde(LED_VERDE);
ControlLed ledAzul(LED_AZUL);
Display display;
ClienteThingSpeak nube(CHANNEL_TS, API_KEY_TS);
// Estado compartido con el bot. Se pasan por referencia: cuando el bot cambia
// el umbral o el modo desde Telegram, el loop() ve el cambio directamente.
//   umbral:   voltaje a partir del cual se enciende el LED azul en modo automático.
//   modoAuto: false = LEDs manejados por comandos; true = LED azul según el potenciómetro.
float umbral = 0.5;
bool modoAuto = false;
// El bot recibe referencias a todos los periféricos para poder leerlos y controlarlos.
BotTelegram telegram(API_KEY_TELEGRAM, sensor, pot, ledVerde, ledAzul,
                     display, nube, umbral, modoAuto);

// ---------------- Control de tiempo con millis() ----------------
// Cada variable guarda el momento (en ms) en que se hizo por última vez esa tarea.
bool telegramConectado = false;
unsigned long ultimaLectura = 0;
unsigned long ultimoIntento = 0;
unsigned long ultimaSubida = 0;

// Prototipos: se anuncian antes para poder usarlas en setup() y loop().
void inicializarPerifericos();
bool conectarWiFi();
void comprobarSensores();
void actualizarControlAutomatico();
void enviarTelemetriaPeriodica();

// Se ejecuta una sola vez al encender la placa.
void setup() {
  Serial.begin(9600);
  inicializarPerifericos();
  nube.iniciar();
  // Telegram necesita Internet, así que solo se inicia si hay WiFi.
  if (conectarWiFi()) {
    telegramConectado = telegram.iniciar();
    display.mostrar("WiFi conectado", telegramConectado ? "Telegram listo" : "Telegram pendiente");
  }
  // Los intervalos empiezan a contar desde este momento.
  ultimoIntento = millis();
  ultimaSubida = millis();
}

// Se repite continuamente. Ninguna tarea bloquea: cada una revisa con millis()
// si ya pasó su intervalo y, si no, sale enseguida.
void loop() {
  comprobarSensores();

  // Si se perdió el WiFi, también se perdió Telegram.
  if (!wifi.connected()) {
    telegramConectado = false;
  }
  // Reconexión automática: cada INTERVAL_RECONEXION se reintenta WiFi y luego Telegram.
  if (!telegramConectado && millis() - ultimoIntento >= INTERVAL_RECONEXION) {
    ultimoIntento = millis();
    if (wifi.connected() || conectarWiFi()) {
      telegramConectado = telegram.iniciar();
      display.mostrar("WiFi conectado", telegramConectado ? "Telegram listo" : "Telegram pendiente");
    }
  }
  // Revisa si llegaron mensajes nuevos al bot.
  if (telegramConectado) {
    telegram.actualizar();
  }
  // Se evalúa después de Telegram para aplicar enseguida cambios de modo o de umbral.
  actualizarControlAutomatico();
  enviarTelemetriaPeriodica();
}

// Configura LEDs, potenciómetro, DHT y la pantalla OLED.
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

// Intenta conectarse al WiFi e informa el resultado por Serial y en pantalla.
// Devuelve true si logró conectarse.
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

// Cada INTERVAL_SENSORES imprime por Serial temperatura, humedad y voltaje del potenciómetro.
void comprobarSensores() {
  // Si todavía no pasó el intervalo no hace nada (es un "delay" que no bloquea).
  if (millis() - ultimaLectura < INTERVAL_SENSORES) {
    return;
  }
  ultimaLectura = millis();
  const float temperatura = sensor.temperatura();
  const float humedad = sensor.humedad();
  // El DHT devuelve NaN ("no es un número") cuando la lectura falla.
  if (isnan(temperatura) || isnan(humedad)) {
    Serial.println("Error de lectura del DHT22.");
  } else {
    // En printf, %% imprime un signo % literal.
    Serial.printf("DHT22 | Temperatura: %.1f C | Humedad: %.1f %%\r\n", temperatura, humedad);
  }
  Serial.printf("Potenciometro GPIO 32: %.2f V\r\n", pot.leerVoltaje());
}

// Modo automático: enciende el LED azul si el potenciómetro supera el umbral
// (misma idea que el relé de la Práctica 1). En modo manual no hace nada.
void actualizarControlAutomatico() {
  if (modoAuto) {
    if (pot.leerVoltaje() > umbral) {
      ledAzul.encender();
    } else {
      ledAzul.apagar();
    }
  }
}

// Cada INTERVAL_THINGSPEAK sube temperatura y humedad a ThingSpeak.
void enviarTelemetriaPeriodica() {
  // Sale si no hay WiFi, si no pasó el intervalo o si ThingSpeak todavía no acepta otro envío.
  if (!wifi.connected() || millis() - ultimaSubida < INTERVAL_THINGSPEAK || !nube.disponible()) {
    return;
  }
  ultimaSubida = millis();
  const float temperatura = sensor.temperatura();
  const float humedad = sensor.humedad();
  // No se suben lecturas inválidas a la nube.
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
