#include "Display.h"

namespace {
const short ANCHO = 128;
const short ALTO = 64;
const uint8_t DIRECCION_I2C = 0x3c;
// 6 px por letra -> 21 por línea
const short MAX_CARACTERES = 21;
// Posiciones en Y (px)
const short Y_LINEA_1 = 0;
const short Y_LINEA_2 = 8;
const short Y_SEPARADOR = 32;
const short Y_HISTORIAL = 36;
const short ALTO_RENGLON = 9;
const short CANT_HISTORIAL = 3;

String limitarLinea(String texto) {
  texto.replace('\r', ' ');
  texto.replace('\n', ' ');
  return texto.substring(0, MAX_CARACTERES);
}
}

// -1: sin pin de reset
Display::Display() : oled(ANCHO, ALTO, &Wire, -1) {}

bool Display::iniciar(short sda, short scl) {
  Wire.begin(sda, scl);
  iniciada = oled.begin(DIRECCION_I2C, true);
  if (iniciada) {
    redibujar();
  }
  return iniciada;
}

void Display::dibujarBuffer() {
  for (int i = 0; i < CANT_HISTORIAL; i++) {
    oled.setCursor(0, Y_HISTORIAL + i * ALTO_RENGLON);
    oled.print(ultimos[i]);
  }
}

void Display::redibujar() {
  if (!iniciada) return;

  oled.clearDisplay();
  oled.setTextSize(1);
  oled.setTextColor(SH110X_WHITE);
  oled.setTextWrap(false);
  oled.setCursor(0, Y_LINEA_1);
  oled.print(lineaSuperior1);
  oled.setCursor(0, Y_LINEA_2);
  oled.print(lineaSuperior2);
  oled.drawLine(0, Y_SEPARADOR, ANCHO - 1, Y_SEPARADOR, SH110X_WHITE);
  dibujarBuffer();
  // Sin esto no se ve nada
  oled.display();
}

void Display::mostrar(const String &linea1, const String &linea2) {
  lineaSuperior1 = limitarLinea(linea1);
  lineaSuperior2 = limitarLinea(linea2);
  redibujar();
}

// Corre el historial y se pierde el más viejo
void Display::registrarComando(const String &cmd) {
  ultimos[2] = ultimos[1];
  ultimos[1] = ultimos[0];
  ultimos[0] = limitarLinea(cmd);
  redibujar();
}
