#include "BotTelegram.h"
#include <time.h>

BotTelegram::BotTelegram(const char *token) : bot(token, client) {}

bool BotTelegram::iniciar() {
  if (WiFi.status() != WL_CONNECTED) {
    return false;
  }

  client.setCACert(TELEGRAM_CERTIFICATE_ROOT);

  // EchoBot sincroniza la hora para validar el certificado HTTPS.
  configTime(0, 0, "pool.ntp.org");
  const unsigned long inicio = millis();
  while (time(nullptr) < 24 * 3600) {
    if (millis() - inicio >= 15000 || WiFi.status() != WL_CONNECTED) {
      Serial.println("No se pudo sincronizar la hora para Telegram.");
      return false;
    }
    delay(100);
  }

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

void BotTelegram::procesarMensajes(int cantidad) {
  for (int i = 0; i < cantidad; i++) {
    const String chatId = bot.messages[i].chat_id;
    const String texto = bot.messages[i].text;
    Serial.println("Mensaje recibido: " + texto);

    // Prueba de ida y vuelta del ejemplo EchoBot incluido en el material.
    if (!bot.sendMessage(chatId, texto, "")) {
      Serial.println("No se pudo responder al mensaje de Telegram.");
    }
  }
}
