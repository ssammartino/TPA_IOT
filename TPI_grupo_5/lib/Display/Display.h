#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

class Display {
  public:
    Display();
    void iniciar();
    void mostrar(const String &linea1, const String &linea2);
    void registrarComando(const String &cmd);

  private:
    Adafruit_SSD1306 oled;
    String ultimos[3] = {"", "", ""};
    void dibujarBuffer();
};

#endif