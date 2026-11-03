#include "BotTelegram.h"
#include <time.h>

// Constructor: solo guarda las referencias recibidas (lista de inicialización).
// El bot se crea con el token y el cliente seguro, como en la Práctica 6.
BotTelegram::BotTelegram(const char *token, SensorDHT &sensor, Potenciometro &pot,
                         ControlLed &ledVerde, ControlLed &ledAzul, Display &display,
                         ClienteThingSpeak &thingspeak, float &umbralActual, bool &automatico)
    : bot(token, client), dht(sensor), pot(pot), ledVerde(ledVerde), ledAzul(ledAzul),
      display(display), thingspeak(thingspeak), umbral(umbralActual), modoAuto(automatico) {}

bool BotTelegram::iniciar() {
  if (WiFi.status() != WL_CONNECTED) {
    return false;
  }

  // Certificado raíz de Telegram para validar la conexión HTTPS (igual que en la Práctica 6).
  client.setCACert(TELEGRAM_CERTIFICATE_ROOT);

  // EchoBot sincroniza la hora para validar el certificado HTTPS.
  // Se espera como máximo 15 s a que el reloj tenga una fecha real.
  configTime(0, 0, "pool.ntp.org");
  const unsigned long inicio = millis();
  while (time(nullptr) < 24 * 3600) {
    if (millis() - inicio >= 15000 || WiFi.status() != WL_CONNECTED) {
      Serial.println("No se pudo sincronizar la hora para Telegram.");
      return false;
    }
    delay(100);
  }

  // getMe() prueba que el token sea válido y que Telegram responda.
  if (!bot.getMe()) {
    Serial.println("No se pudo conectar con Telegram. Revisar token y acceso a Internet.");
    return false;
  }

  // Al arrancar se omiten mensajes viejos; las reconexiones conservan pendientes.
  if (primeraConexion) {
    int pendientes = bot.getUpdates(bot.last_message_received + 1);
    while (pendientes > 0) {
      pendientes = bot.getUpdates(bot.last_message_received + 1);
    }
    primeraConexion = false;
  }

  Serial.println("Telegram conectado: @" + bot.userName);
  ant = millis();
  return true;
}

// Mismo esquema que el loop() de la Práctica 6: cada INTERVAL ms pide los
// mensajes nuevos y los procesa hasta que no quede ninguno pendiente.
void BotTelegram::actualizar() {
  if (WiFi.status() != WL_CONNECTED || millis() - ant < INTERVAL) {
    return;
  }
  ant = millis();

  int cantidad = bot.getUpdates(bot.last_message_received + 1);
  while (cantidad > 0) {
    procesarMensajes(cantidad);
    cantidad = bot.getUpdates(bot.last_message_received + 1);
  }
}
// Recorre los mensajes recibidos; trim() quita espacios y saltos de línea sobrantes.
void BotTelegram::procesarMensajes(int cantidad) {
  for (int i = 0; i < cantidad; i++) {
    const String chatId = bot.messages[i].chat_id;
    String texto = bot.messages[i].text;
    texto.trim();
    Serial.println("Mensaje recibido: " + texto);
    procesarComando(chatId, texto);
  }
}

// Decide qué hacer según el texto recibido y llama al método del comando.
void BotTelegram::procesarComando(const String &chatId, const String &texto) {
  bool exitoso = false;
  if (texto == "/start") {
    cmdStart(chatId);
    exitoso = true;
  } else if (texto == "/umbral" || texto.startsWith("/umbral ")) {
    exitoso = cmdUmbral(chatId, texto);
  } else if (texto == "/estado") {
    exitoso = cmdEstado(chatId);
  } else if (texto == "/autoon") {
    cmdAuto(chatId, true);
    exitoso = true;
  } else if (texto == "/autooff") {
    cmdAuto(chatId, false);
    exitoso = true;
  } else if (texto == "/thingup") {
    exitoso = cmdThingUp(chatId);
  } else if (texto.startsWith("/display")) {
    // substring(8) se queda con lo que viene después de "/display" (8 caracteres): led, pot o dht.
    String sub = texto.substring(8);
    sub.trim();
    exitoso = cmdDisplay(chatId, sub);
  } else if (texto == "/verdeon") {
    exitoso = cmdLed(chatId, ledVerde, "verde", true);
  } else if (texto == "/verdeoff") {
    exitoso = cmdLed(chatId, ledVerde, "verde", false);
  } else if (texto == "/azulon") {
    exitoso = cmdLed(chatId, ledAzul, "azul", true);
  } else if (texto == "/azuloff") {
    exitoso = cmdLed(chatId, ledAzul, "azul", false);
  } else {
    bot.sendMessage(chatId, "❓ Comando no reconocido. Enviá /start para ver el menú.", "");
  }

  // Los errores y comandos desconocidos no ingresan al historial visual.
  if (exitoso) {
    display.registrarComando(texto);
  }
}

// /start: envía el menú de comandos. Se usa formato HTML (<b>, <i>), por eso
// los signos < y > del texto se escriben como &lt; y &gt;.
void BotTelegram::cmdStart(const String &chatId) {
  String menu = "🤖 <b>TPI Grupo 5 - ESP32</b>\n\n";
  menu += "📊 <b>Monitoreo</b>\n";
  menu += "/estado - Temperatura, humedad, potenciómetro y LEDs\n\n";
  menu += "⚙️ <b>Control automático</b>\n";
  menu += "/umbral &lt;valor&gt; - Umbral entre 1 y 1.5 V (defecto 0.5 V)\n";
  menu += "/autoon - Toma el voltaje actual como referencia y activa el modo automatico\n";
  menu += "/autooff - Pasa a modo manual\n\n";
  menu += "💡 <b>Control manual</b>\n";
  menu += "/verdeon - /verdeoff - LED verde\n";
  menu += "/azulon - /azuloff - LED azul\n\n";
  menu += "☁️ <b>Nube</b>\n";
  menu += "/thingup - Envía temperatura y humedad a ThingSpeak\n\n";
  menu += "Tambien se envian automaticamente cada 20 s cuando hay conexion.\n\n";
  menu += "🖥️ <b>Display OLED</b>\n";
  menu += "/displayled - Estado de los LEDs\n";
  menu += "/displaypot - Voltaje del potenciómetro\n";
  menu += "/displaydht - Temperatura y humedad\n\n";
  menu += "✍️ <i>Escribí los comandos como texto plano.</i>";
  bot.sendMessage(chatId, menu, "HTML");
}

// /umbral <valor>: cambia el umbral si el valor es un número entre 1 y 1.5 V.
// Si no es válido, vuelve al valor por defecto (0.5 V).
bool BotTelegram::cmdUmbral(const String &chatId, const String &texto) {
  // substring(7) quita "/umbral" y deja solo el número. Se acepta coma decimal.
  String valor = texto.substring(7);
  valor.trim();
  valor.replace(',', '.');

  // toFloat acepta prefijos numericos: se comprueba primero todo el argumento.
  // (por ejemplo "1.2abc" daría 1.2, así que se revisa carácter por carácter).
  bool formatoValido = valor.length() > 0;
  bool tieneDigitos = false;
  bool tienePunto = false;
  for (unsigned int i = 0; i < valor.length(); i++) {
    const char c = valor.charAt(i);
    if (c >= '0' && c <= '9') {
      tieneDigitos = true;
    } else if (c == '.' && !tienePunto) {
      tienePunto = true;
    } else if (i == 0 && (c == '+' || c == '-')) {
      // El rango se comprueba despues de convertir.
    } else {
      formatoValido = false;
    }
  }
  const float nuevo = valor.toFloat();
  if (formatoValido && tieneDigitos && nuevo >= UMBRAL_MIN && nuevo <= UMBRAL_MAX) {
    umbral = nuevo;
    bot.sendMessage(chatId, "Umbral asignado: " + String(umbral, 2) + " V", "");
    return true;
  }
  umbral = UMBRAL_DEFECTO;
  bot.sendMessage(chatId, "Valor invalido o fuera de rango [1; 1.5] V. Umbral restablecido a 0.5 V.", "");
  return false;
}

// /estado: arma un resumen con todas las lecturas y estados y lo envía.
bool BotTelegram::cmdEstado(const String &chatId) {
  float temperatura = dht.temperatura();
  float humedad = dht.humedad();

  String msg = "📊 <b>Estado actual</b>\n\n";
  if (isnan(temperatura) || isnan(humedad)) {
    msg += "🌡️ DHT22: error de lectura\n";
  } else {
    msg += "🌡️ Temperatura: " + String(temperatura, 1) + " °C\n";
    msg += "💧 Humedad: " + String(humedad, 1) + " %\n";
  }
  msg += "🎚️ Potenciómetro: " + String(pot.leerVoltaje(), 2) + " V\n";
  msg += "📏 Umbral: " + String(umbral, 2) + " V\n";
  msg += "🟢 LED verde: " + ledVerde.estado() + "\n";
  msg += "🔵 LED azul: " + ledAzul.estado() + "\n";
  msg += "⚙️ Modo: " + String(modoAuto ? "automático" : "manual");
  bot.sendMessage(chatId, msg, "HTML");
  return !isnan(temperatura) && !isnan(humedad);
}

// /autoon y /autooff. Al activar, el voltaje actual del potenciómetro pasa a ser el umbral.
// En ambos casos se apaga el LED azul para empezar desde un estado conocido.
void BotTelegram::cmdAuto(const String &chatId, bool activar) {
  if (activar) {
    umbral = pot.leerVoltaje();
    modoAuto = true;
    ledAzul.apagar();
    bot.sendMessage(chatId, "Modo automatico activado. Referencia tomada del potenciometro: " +
                    String(umbral, 2) + " V. /umbral permite ajustarla.", "");
  } else {
    modoAuto = false;
    ledAzul.apagar();
    bot.sendMessage(chatId, "Modo manual activado. Los LEDs responden a comandos de Telegram.", "");
  }
}

// /thingup: envío manual a ThingSpeak, respetando el tiempo mínimo entre envíos.
bool BotTelegram::cmdThingUp(const String &chatId) {
  const float temperatura = dht.temperatura();
  const float humedad = dht.humedad();
  if (isnan(temperatura) || isnan(humedad)) {
    bot.sendMessage(chatId, "Error leyendo el DHT22, no se enviaron datos.", "");
    return false;
  }
  if (!thingspeak.disponible()) {
    // Convierte ms a segundos redondeando hacia arriba.
    const unsigned long segundos = (thingspeak.esperaRestante() + 999) / 1000;
    bot.sendMessage(chatId, "Espera " + String(segundos) + " s antes de otro envio a ThingSpeak.", "");
    return false;
  }
  if (!thingspeak.enviar(temperatura, humedad)) {
    bot.sendMessage(chatId, "Fallo el envio a ThingSpeak. Codigo: " + String(thingspeak.ultimoCodigo()), "");
    return false;
  }
  bot.sendMessage(chatId, "Enviado a ThingSpeak: " + String(temperatura, 1) + " C, " +
                  String(humedad, 1) + " %", "");
  return true;
}

// /displayled, /displaypot, /displaydht: muestra el dato pedido en la parte superior del OLED.
bool BotTelegram::cmdDisplay(const String &chatId, const String &sub) {
  if (sub == "led") {
    display.mostrar("Led verde " + ledVerde.estado(), "Led azul " + ledAzul.estado());
  } else if (sub == "pot") {
    display.mostrar("Pot: " + String(pot.leerVoltaje(), 2) + " V", "Umbral: " + String(umbral, 2) + " V");
  } else if (sub == "dht") {
    float temperatura = dht.temperatura();
    float humedad = dht.humedad();
    if (isnan(temperatura) || isnan(humedad)) {
      display.mostrar("DHT22:", "error de lectura");
      bot.sendMessage(chatId, "No se pudo leer el DHT22.", "");
      return false;
    } else {
      display.mostrar("Temp: " + String(temperatura, 1) + " C", "Hum: " + String(humedad, 1) + " %");
    }
  } else {
    display.mostrar("Comando no", "identificado: " + sub);
    bot.sendMessage(chatId, "⚠️ /display" + sub + " no es un comando identificado.", "");
    return false;
  }
  bot.sendMessage(chatId, "🖥️ Mostrando en el display: " + sub, "");
  return true;
}

// Enciende o apaga el LED indicado. El LED azul no se puede tocar a mano
// mientras está en modo automático (lo controla el potenciómetro).
bool BotTelegram::cmdLed(const String &chatId, ControlLed &led, const String &nombre, bool encender) {
  // &led == &ledAzul compara direcciones de memoria: ¿es el mismo objeto LED azul?
  if (modoAuto && &led == &ledAzul) {
    bot.sendMessage(chatId, "⚠️ El LED azul está en modo automático. Enviá /autooff primero.", "");
    return false;
  }
  encender ? led.encender() : led.apagar();
  bot.sendMessage(chatId, "💡 LED " + nombre + " " + led.estado(), "");
  return true;
}
