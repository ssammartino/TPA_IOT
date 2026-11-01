#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <UniversalTelegramBot.h>
#include <DHT.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ---------------- Pines ----------------
const short LED_VERDE = 23;
const short LED_AZUL = 2;
const short PIN_DHT = 33;
const short PIN_POT = 32;

// ---------------- Credenciales ----------------
const char *SSID = "Wokwi-GUEST"; // Red WiFi
const char *PASS = "";
const char *API_KEY_TELEGRAM = "8931640257:AAHYxnhJPxTUbLoU6N5WyVL5yoZpiE8moMI";
const char *API_KEY_THINGSPEAK = "TU_WRITE_API_KEY"; // Write API Key del canal

// ---------------- Configuración ----------------
const float UMBRAL_DEFECTO = 0.5;
const float UMBRAL_MIN = 1.0;
const float UMBRAL_MAX = 1.5;
const unsigned long INTERVALO_BOT_MS = 1000;
const int ANCHO_OLED = 128;
const int ALTO_OLED = 64;
const int CANT_HISTORIAL = 3;

// ---------------- Objetos ----------------
WiFiClientSecure client;
UniversalTelegramBot bot(API_KEY_TELEGRAM, client);
DHT dht(PIN_DHT, DHT22);
Adafruit_SSD1306 display(ANCHO_OLED, ALTO_OLED, &Wire, -1);

// ---------------- Estado ----------------
float umbral = UMBRAL_DEFECTO;
bool modoAuto = false;
unsigned long ultimoChequeoBot = 0;
String historial[CANT_HISTORIAL]; // últimos comandos exitosos, [0] = más reciente
String lineaEstado1 = "Esperando";
String lineaEstado2 = "comando...";

// ---------------- Prototipos ----------------
void conectarWiFi();
void procesarMensajes(int cantidad);
void procesarComando(const String &chatId, const String &texto);
void cmdStart(const String &chatId);
void cmdUmbral(const String &chatId, const String &texto);
void cmdEstado(const String &chatId);
void cmdAuto(const String &chatId, bool activar);
void cmdThingUp(const String &chatId);
void cmdDisplay(const String &chatId, const String &sub);
void cmdLed(const String &chatId, short pin, bool encender);
void controlAutomatico();
float leerVoltajePot();
bool enviarThingSpeak(float temperatura, float humedad);
void registrarComando(const String &cmd);
void mostrarEnDisplay(const String &l1, const String &l2);
void refrescarDisplay();
String estadoLed(short pin);

void setup() {
  Serial.begin(9600);
  pinMode(LED_VERDE, OUTPUT);
  pinMode(LED_AZUL, OUTPUT);
  dht.begin();

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("No se encontro el OLED");
  }
  refrescarDisplay();

  conectarWiFi();
  client.setCACert(TELEGRAM_CERTIFICATE_ROOT);
}

void loop() {
  if (millis() - ultimoChequeoBot > INTERVALO_BOT_MS) {
    int cantidad = bot.getUpdates(bot.last_message_received + 1);
    while (cantidad) {
      procesarMensajes(cantidad);
      cantidad = bot.getUpdates(bot.last_message_received + 1);
    }
    ultimoChequeoBot = millis();
  }

  if (modoAuto) {
    controlAutomatico();
  }
}

// ================= WiFi =================
void conectarWiFi() {
  WiFi.begin(SSID, PASS);
  // Bucle bloqueante hasta establecer conexión WiFi
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi conectado");
  Serial.println(WiFi.localIP()); // Muestra la IP asignada por el DHCP
}

// ================= Bot de Telegram =================
void procesarMensajes(int cantidad) {
  for (int i = 0; i < cantidad; i++) {
    String chatId = bot.messages[i].chat_id;
    String texto = bot.messages[i].text;
    texto.trim();
    Serial.println("Recibido: " + texto);
    procesarComando(chatId, texto);
  }
}

void procesarComando(const String &chatId, const String &texto) {
  if (texto == "/start") {
    cmdStart(chatId);
  } else if (texto.startsWith("/umbral")) {
    cmdUmbral(chatId, texto);
  } else if (texto == "/estado") {
    cmdEstado(chatId);
  } else if (texto == "/autoon") {
    cmdAuto(chatId, true);
  } else if (texto == "/autooff") {
    cmdAuto(chatId, false);
  } else if (texto == "/thingup") {
    cmdThingUp(chatId);
  } else if (texto.startsWith("/display")) {
    cmdDisplay(chatId, texto.substring(8));
  } else if (texto == "/verdeon") {
    cmdLed(chatId, LED_VERDE, true);
  } else if (texto == "/verdeoff") {
    cmdLed(chatId, LED_VERDE, false);
  } else if (texto == "/azulon") {
    cmdLed(chatId, LED_AZUL, true);
  } else if (texto == "/azuloff") {
    cmdLed(chatId, LED_AZUL, false);
  } else {
    bot.sendMessage(chatId, "❓ Comando no reconocido. Enviá /start para ver el menú.", "");
  }
}

void cmdStart(const String &chatId) {
  String menu = "🤖 <b>TPI Grupo 5 - ESP32</b>\n\n";
  menu += "📊 <b>Monitoreo</b>\n";
  menu += "/estado - Temperatura, humedad, potenciómetro y LEDs\n\n";
  menu += "⚙️ <b>Control automático</b>\n";
  menu += "/umbral &lt;valor&gt; - Umbral entre 1 y 1.5 V (defecto 0.5 V)\n";
  menu += "/autoon - Activa el modo automático (LED azul)\n";
  menu += "/autooff - Pasa a modo manual\n\n";
  menu += "💡 <b>Control manual</b>\n";
  menu += "/verdeon - /verdeoff - LED verde\n";
  menu += "/azulon - /azuloff - LED azul\n\n";
  menu += "☁️ <b>Nube</b>\n";
  menu += "/thingup - Envía temperatura y humedad a ThingSpeak\n\n";
  menu += "🖥️ <b>Display OLED</b>\n";
  menu += "/displayled - Estado de los LEDs\n";
  menu += "/displaypot - Voltaje del potenciómetro\n";
  menu += "/displaydht - Temperatura y humedad\n\n";
  menu += "✍️ <i>Escribí los comandos como texto plano.</i>";
  bot.sendMessage(chatId, menu, "HTML");
  registrarComando("/start");
}

void cmdUmbral(const String &chatId, const String &texto) {
  String valor = texto.substring(7);
  valor.trim();
  valor.replace(',', '.');
  float nuevo = valor.toFloat();

  if (valor.length() > 0 && nuevo >= UMBRAL_MIN && nuevo <= UMBRAL_MAX) {
    umbral = nuevo;
    bot.sendMessage(chatId, "✅ Umbral asignado: " + String(umbral, 2) + " V", "");
  } else {
    umbral = UMBRAL_DEFECTO;
    bot.sendMessage(chatId, "⚠️ Valor fuera de rango [1; 1.5] V. Se restablece el umbral a " +
                                String(umbral, 2) + " V", "");
  }
  registrarComando("/umbral " + String(umbral, 2));
}

void cmdEstado(const String &chatId) {
  float temperatura = dht.readTemperature();
  float humedad = dht.readHumidity();

  String msg = "📊 <b>Estado actual</b>\n\n";
  if (isnan(temperatura) || isnan(humedad)) {
    msg += "🌡️ DHT22: error de lectura\n";
  } else {
    msg += "🌡️ Temperatura: " + String(temperatura, 1) + " °C\n";
    msg += "💧 Humedad: " + String(humedad, 1) + " %\n";
  }
  msg += "🎚️ Potenciómetro: " + String(leerVoltajePot(), 2) + " V\n";
  msg += "📏 Umbral: " + String(umbral, 2) + " V\n";
  msg += "🟢 LED verde: " + estadoLed(LED_VERDE) + "\n";
  msg += "🔵 LED azul: " + estadoLed(LED_AZUL) + "\n";
  msg += "⚙️ Modo: " + String(modoAuto ? "automático" : "manual");
  bot.sendMessage(chatId, msg, "HTML");
  registrarComando("/estado");
}

void cmdAuto(const String &chatId, bool activar) {
  modoAuto = activar;
  if (activar) {
    bot.sendMessage(chatId, "🤖 Modo automático activado. El LED azul se enciende cuando el potenciómetro supera " +
                                String(umbral, 2) + " V", "");
    registrarComando("/autoon");
  } else {
    digitalWrite(LED_AZUL, LOW);
    bot.sendMessage(chatId, "✋ Modo manual activado. Los LEDs solo responden a comandos de Telegram.", "");
    registrarComando("/autooff");
  }
}

void cmdThingUp(const String &chatId) {
  float temperatura = dht.readTemperature();
  float humedad = dht.readHumidity();

  if (isnan(temperatura) || isnan(humedad)) {
    bot.sendMessage(chatId, "❌ Error leyendo el DHT22, no se envió nada.", "");
    return;
  }
  if (enviarThingSpeak(temperatura, humedad)) {
    bot.sendMessage(chatId, "☁️ Enviado a ThingSpeak: " + String(temperatura, 1) + " °C, " +
                                String(humedad, 1) + " %", "");
    registrarComando("/thingup");
  } else {
    bot.sendMessage(chatId, "❌ Falló el envío a ThingSpeak.", "");
  }
}

void cmdDisplay(const String &chatId, const String &sub) {
  if (sub == "led") {
    mostrarEnDisplay("Led verde " + estadoLed(LED_VERDE), "Led azul " + estadoLed(LED_AZUL));
  } else if (sub == "pot") {
    mostrarEnDisplay("Pot: " + String(leerVoltajePot(), 2) + " V", "Umbral: " + String(umbral, 2) + " V");
  } else if (sub == "dht") {
    float temperatura = dht.readTemperature();
    float humedad = dht.readHumidity();
    if (isnan(temperatura) || isnan(humedad)) {
      mostrarEnDisplay("DHT22:", "error de lectura");
    } else {
      mostrarEnDisplay("Temp: " + String(temperatura, 1) + " C", "Hum: " + String(humedad, 1) + " %");
    }
  } else {
    mostrarEnDisplay("Comando no", "identificado: " + sub);
    bot.sendMessage(chatId, "⚠️ /display" + sub + " no es un comando identificado.", "");
    return;
  }
  bot.sendMessage(chatId, "🖥️ Mostrando en el display: " + sub, "");
  registrarComando("/display" + sub);
}

void cmdLed(const String &chatId, short pin, bool encender) {
  if (modoAuto && pin == LED_AZUL) {
    bot.sendMessage(chatId, "⚠️ El LED azul está en modo automático. Enviá /autooff primero.", "");
    return;
  }
  digitalWrite(pin, encender ? HIGH : LOW);
  String nombre = (pin == LED_VERDE) ? "verde" : "azul";
  bot.sendMessage(chatId, "💡 LED " + nombre + " " + estadoLed(pin), "");
  registrarComando("/" + nombre + (encender ? "on" : "off"));
}

// ================= Sensores y actuadores =================
void controlAutomatico() {
  digitalWrite(LED_AZUL, leerVoltajePot() > umbral ? HIGH : LOW);
}

float leerVoltajePot() {
  return analogRead(PIN_POT) * 3.3 / 4095.0; // ADC de 12 bits -> 0 a 3.3 V
}

String estadoLed(short pin) {
  return digitalRead(pin) ? "encendido" : "apagado";
}

// ================= ThingSpeak =================
bool enviarThingSpeak(float temperatura, float humedad) {
  HTTPClient http;
  String url = "http://api.thingspeak.com/update?api_key=" + String(API_KEY_THINGSPEAK) +
               "&field1=" + String(temperatura, 1) + "&field2=" + String(humedad, 1);
  http.begin(url);
  int codigo = http.GET();
  String respuesta = http.getString();
  http.end();
  // ThingSpeak devuelve el id de la entrada, o "0" si la rechazó
  return codigo == 200 && respuesta.toInt() > 0;
}

// ================= Display OLED =================
void registrarComando(const String &cmd) {
  for (int i = CANT_HISTORIAL - 1; i > 0; i--) {
    historial[i] = historial[i - 1];
  }
  historial[0] = cmd;
  refrescarDisplay();
}

void mostrarEnDisplay(const String &l1, const String &l2) {
  lineaEstado1 = l1;
  lineaEstado2 = l2;
  refrescarDisplay();
}

// Parte superior: estado solicitado. Parte inferior: últimos 3 comandos.
void refrescarDisplay() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(0, 0);
  display.println(lineaEstado1);
  display.println(lineaEstado2);

  display.drawLine(0, 30, ANCHO_OLED - 1, 30, SSD1306_WHITE);
  display.setCursor(0, 34);
  for (int i = 0; i < CANT_HISTORIAL; i++) {
    if (historial[i].length() > 0) {
      display.println("> " + historial[i]);
    }
  }
  display.display();
}
