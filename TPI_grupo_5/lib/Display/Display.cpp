#include "Display.h"

Display::Display() : oled(128, 64, &Wire, -1) {}

void Display::iniciar() {
  oled.begin(SSD1306_SWITCHCAPVCC, 0x3C);
}

void Display::dibujarBuffer() {
  for (int i = 0; i < 3; i++) {
    oled.setCursor(0, 36 + i * 9);
    oled.println(ultimos[i]);
  }
}

void Display::mostrar(const String &linea1, const String &linea2) {
  oled.clearDisplay();
  oled.setTextSize(1);
  oled.setTextColor(SSD1306_WHITE);
  oled.setCursor(0, 0);
  oled.println(linea1);
  oled.println(linea2);
  oled.drawLine(0, 32, 127, 32, SSD1306_WHITE);
  dibujarBuffer();
  oled.display();
}

void Display::registrarComando(const String &cmd) {
  ultimos[2] = ultimos[1];
  ultimos[1] = ultimos[0];
  ultimos[0] = cmd;
  dibujarBuffer();
  oled.display();
}