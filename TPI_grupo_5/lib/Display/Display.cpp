#include "Display.h"

namespace {
String limitarLinea(String texto) {
  texto.replace('\r', ' ');
  texto.replace('\n', ' ');
  return texto.substring(0, 21);
}
}

// Controlador y resolucion del ejemplo OLED de Clase03.
Display::Display() : oled(128, 64, &Wire, -1) {}

bool Display::iniciar(short sda, short scl) {
  Wire.begin(sda, scl);
  iniciada = oled.begin(0x3c, true);
  if (iniciada) {
    redibujar();
  }
  return iniciada;
}

void Display::dibujarBuffer() {
  for (int i = 0; i < 3; i++) {
    oled.setCursor(0, 36 + i * 9);
    oled.print(ultimos[i]);
  }
}

void Display::redibujar() {
  if (!iniciada) return;

  // Limpiar todo evita restos cuando el siguiente comando es mas corto.
  oled.clearDisplay();
  oled.setTextSize(1);
  oled.setTextColor(SH110X_WHITE);
  oled.setTextWrap(false);
  oled.setCursor(0, 0);
  oled.print(lineaSuperior1);
  oled.setCursor(0, 8);
  oled.print(lineaSuperior2);
  oled.drawLine(0, 32, 127, 32, SH110X_WHITE);
  dibujarBuffer();
  oled.display();
}

void Display::mostrar(const String &linea1, const String &linea2) {
  lineaSuperior1 = limitarLinea(linea1);
  lineaSuperior2 = limitarLinea(linea2);
  redibujar();
}

void Display::registrarComando(const String &cmd) {
  ultimos[2] = ultimos[1];
  ultimos[1] = ultimos[0];
  ultimos[0] = limitarLinea(cmd);
  redibujar();
}
