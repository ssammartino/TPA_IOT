#include "BotTelegram.h"

BotTelegram::BotTelegram(const char *token, SensorDHT &sensor,
                         float &umbralActual, bool &automatico,
                         short pinVerde, short pinAzul)
    : bot(token, client), dht(sensor), umbral(umbralActual),
      modoAuto(automatico) {
  LED_VERDE = pinVerde;
  LED_AZUL = pinAzul;
}

void BotTelegram::iniciar() {
  client.setCACert(TELEGRAM_CERTIFICATE_ROOT);
}

void BotTelegram::actualizar() {
  if (millis() - ant > INTERVAL) {
    ant = millis();
    int n = bot.getUpdates(bot.last_message_received + 1);
    // Se procesa solamente una cantidad positiva de mensajes.
    while (n > 0) {
      procesarMensajes(n);
      n = bot.getUpdates(bot.last_message_received + 1);
    }
  }
}

// ================= Bot de Telegram =================
void BotTelegram::procesarMensajes(int cantidad) {
  for (int i = 0; i < cantidad; i++) {
    String chatId = bot.messages[i].chat_id;
    String texto = bot.messages[i].text;
    texto.trim();
    Serial.println("Recibido: " + texto);
    procesarComando(chatId, texto);
  }
}

void BotTelegram::procesarComando(const String &chatId, const String &texto) {
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

void BotTelegram::cmdStart(const String &chatId) {
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

void BotTelegram::cmdUmbral(const String &chatId, const String &texto) {
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

void BotTelegram::cmdEstado(const String &chatId) {
  float temperatura = dht.temperatura();
  float humedad = dht.humedad();

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

void BotTelegram::cmdAuto(const String &chatId, bool activar) {
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

void BotTelegram::cmdThingUp(const String &chatId) {
  float temperatura = dht.temperatura();
  float humedad = dht.humedad();

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

void BotTelegram::cmdDisplay(const String &chatId, const String &sub) {
  if (sub == "led") {
    mostrarEnDisplay("Led verde " + estadoLed(LED_VERDE), "Led azul " + estadoLed(LED_AZUL));
  } else if (sub == "pot") {
    mostrarEnDisplay("Pot: " + String(leerVoltajePot(), 2) + " V", "Umbral: " + String(umbral, 2) + " V");
  } else if (sub == "dht") {
    float temperatura = dht.temperatura();
    float humedad = dht.humedad();
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

void BotTelegram::cmdLed(const String &chatId, short pin, bool encender) {
  if (modoAuto && pin == LED_AZUL) {
    bot.sendMessage(chatId, "⚠️ El LED azul está en modo automático. Enviá /autooff primero.", "");
    return;
  }
  digitalWrite(pin, encender ? HIGH : LOW);
  String nombre = (pin == LED_VERDE) ? "verde" : "azul";
  bot.sendMessage(chatId, "💡 LED " + nombre + " " + estadoLed(pin), "");
  registrarComando("/" + nombre + (encender ? "on" : "off"));
}
