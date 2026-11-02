#include "BotTelegram.h"

BotTelegram::BotTelegram(const char *token, SensorDHT &sensor, Potenciometro &pot,
                          ControlLed &ledVerde, ControlLed &ledAzul,
                          Display &display, ClienteThingSpeak &thingspeak,
                          float &umbralActual, bool &automatico)
    : bot(token, client), dht(sensor), pot(pot), ledVerde(ledVerde),
      ledAzul(ledAzul), display(display), thingspeak(thingspeak),
      umbral(umbralActual), modoAuto(automatico) {}

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

// Aca procesa mensajes, con un ciclo for, donde separa id del chat del mensaje, y envia ambos a procesarComando
void BotTelegram::procesarMensajes(int cantidad) {
  for (int i = 0; i < cantidad; i++) {
    String chatId = bot.messages[i].chat_id;
    String texto = bot.messages[i].text;
    texto.trim();
    Serial.println("Recibido: " + texto);
    procesarComando(chatId, texto);
  }
}

//Aca es donde deriva a la funcion, fijarse si se puede hacer con un case-switch
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
    cmdLed(chatId, ledVerde, "verde", true);
  } else if (texto == "/verdeoff") {
    cmdLed(chatId, ledVerde, "verde", false);
  } else if (texto == "/azulon") {
    cmdLed(chatId, ledAzul, "azul", true);
  } else if (texto == "/azuloff") {
    cmdLed(chatId, ledAzul, "azul", false);
  } else {
    bot.sendMessage(chatId, "❓ Comando no reconocido. Enviá /start para ver el menú.", "");
  }
}

//Muestra menú de comandos
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
  display.registrarComando("/start");
}

//Para asignar un umbral(?
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
  display.registrarComando("/umbral " + String(umbral, 2));
}

//Para corroborar el estado de todo lo que pide: Temperatura, Humedad, Potenciomentro, Umbral, LEDs y el modo
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
  msg += "🎚️ Potenciómetro: " + String(pot.leerVoltaje(), 2) + " V\n";
  msg += "📏 Umbral: " + String(umbral, 2) + " V\n";
  msg += "🟢 LED verde: " + ledVerde.estado() + "\n";
  msg += "🔵 LED azul: " + ledAzul.estado() + "\n";
  msg += "⚙️ Modo: " + String(modoAuto ? "automático" : "manual");
  bot.sendMessage(chatId, msg, "HTML");
  display.registrarComando("/estado");
}

//Para activar o desactivar el modo automatico
void BotTelegram::cmdAuto(const String &chatId, bool activar) {
  modoAuto = activar;
  if (activar) {
    bot.sendMessage(chatId, "🤖 Modo automático activado. El LED azul se enciende cuando el potenciómetro supera " +
                                String(umbral, 2) + " V", "");
    display.registrarComando("/autoon");
  } else {
    ledAzul.apagar();
    bot.sendMessage(chatId, "✋ Modo manual activado. Los LEDs solo responden a comandos de Telegram.", "");
    display.registrarComando("/autooff");
  }
}

//Para enviar temperatura y humedad al canal thingspeak
void BotTelegram::cmdThingUp(const String &chatId) {
  float temperatura = dht.temperatura();
  float humedad = dht.humedad();

  if (isnan(temperatura) || isnan(humedad)) {
    bot.sendMessage(chatId, "❌ Error leyendo el DHT22, no se envió nada.", "");
    return;
  }
  if (thingspeak.enviar(temperatura, humedad)) {
    bot.sendMessage(chatId, "☁️ Enviado a ThingSpeak: " + String(temperatura, 1) + " °C, " +
                                String(humedad, 1) + " %", "");
    display.registrarComando("/thingup");
  } else {
    bot.sendMessage(chatId, "❌ Falló el envío a ThingSpeak.", "");
  }
}


//Para mostrar cosas por el display
void BotTelegram::cmdDisplay(const String &chatId, const String &sub) {
  if (sub == "led") {
    display.mostrar("Led verde " + ledVerde.estado(), "Led azul " + ledAzul.estado());
  } else if (sub == "pot") {
    display.mostrar("Pot: " + String(pot.leerVoltaje(), 2) + " V", "Umbral: " + String(umbral, 2) + " V");
  } else if (sub == "dht") {
    float temperatura = dht.temperatura();
    float humedad = dht.humedad();
    if (isnan(temperatura) || isnan(humedad)) {
      display.mostrar("DHT22:", "error de lectura");
    } else {
      display.mostrar("Temp: " + String(temperatura, 1) + " C", "Hum: " + String(humedad, 1) + " %");
    }
  } else {
    display.mostrar("Comando no", "identificado: " + sub);
    bot.sendMessage(chatId, "⚠️ /display" + sub + " no es un comando identificado.", "");
    return;
  }
  bot.sendMessage(chatId, "🖥️ Mostrando en el display: " + sub, "");
  display.registrarComando("/display" + sub);
}

//Para cambiar el led a on u off
void BotTelegram::cmdLed(const String &chatId, ControlLed &led, const String &nombre, bool encender) {
  if(modoAuto && &led == &ledAzul){
    bot.sendMessage(chatId, "⚠️ El LED azul está en modo automático. Enviá /autooff primero.", "");
    return;
  }
  encender ? led.encender() : led.apagar();
  bot.sendMessage(chatId, "💡 LED " + nombre + " " + led.estado(), "");
  display.registrarComando("/" + nombre + (encender ? "on" : "off"));
}
