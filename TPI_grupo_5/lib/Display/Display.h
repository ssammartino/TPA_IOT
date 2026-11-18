#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

// OLED SH1106 128x64. Arriba dos líneas de mensaje, abajo los últimos
// 3 comandos de Telegram.
class Display {
  public:
    Display();
    bool iniciar(short sda = 21, short scl = 22);
    void mostrar(const String &linea1, const String &linea2 = "");
    void registrarComando(const String &cmd);

  private:
    Adafruit_SH1106G oled;
    // ultimos[0] es el más nuevo
    String ultimos[3] = {"", "", ""};
    String lineaSuperior1;
    String lineaSuperior2;
    bool iniciada = false;
    void redibujar();
    void dibujarBuffer();
};

#endif
