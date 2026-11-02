#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

class Display {
  public:
    Display();
    bool iniciar(short sda = 21, short scl = 22);
    void mostrar(const String &linea1, const String &linea2 = "");
    void registrarComando(const String &cmd);

  private:
    Adafruit_SH1106G oled;
    String ultimos[3] = {"", "", ""};
    String lineaSuperior1;
    String lineaSuperior2;
    bool iniciada = false;
    void redibujar();
    void dibujarBuffer();
};

#endif
