#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

// Pantalla OLED SH1106 de 128x64 por I2C (Prácticas 2 y 3).
// Está dividida en dos zonas:
//   - Arriba: dos líneas de mensaje (mostrar()).
//   - Abajo, debajo de una línea horizontal: los últimos 3 comandos de Telegram
//     ejecutados con éxito (registrarComando()).
class Display {
  public:
    Display();
    // Pines I2C por defecto del ESP32: SDA 21 y SCL 22.
    bool iniciar(short sda = 21, short scl = 22);
    void mostrar(const String &linea1, const String &linea2 = "");
    void registrarComando(const String &cmd);

  private:
    Adafruit_SH1106G oled;
    // Historial de comandos: ultimos[0] es el más reciente.
    String ultimos[3] = {"", "", ""};
    String lineaSuperior1;
    String lineaSuperior2;
    bool iniciada = false;
    void redibujar();
    void dibujarBuffer();
};

#endif
