#include "Display.h"

// Constantes y función auxiliar solo visibles en este archivo (namespace sin nombre).
namespace {
// Pantalla de 128x64 píxeles en la dirección I2C 0x3c.
const short ANCHO = 128;
const short ALTO = 64;
const uint8_t DIRECCION_I2C = 0x3c;
// Con texto de tamaño 1 cada letra ocupa 6 px de ancho: entran 21 por línea.
const short MAX_CARACTERES = 21;
// Posiciones verticales (en px): las dos líneas de mensaje arriba,
// la línea separadora y el historial de comandos debajo.
const short Y_LINEA_1 = 0;
const short Y_LINEA_2 = 8;
const short Y_SEPARADOR = 32;
const short Y_HISTORIAL = 36;
const short ALTO_RENGLON = 9;
const short CANT_HISTORIAL = 3;

// Reemplaza saltos de línea por espacios y recorta a MAX_CARACTERES.
String limitarLinea(String texto) {
  texto.replace('\r', ' ');
  texto.replace('\n', ' ');
  return texto.substring(0, MAX_CARACTERES);
}
}

// Controlador y resolucion del ejemplo OLED de Clase03.
// Bus I2C (Wire) y sin pin de reset (-1).
Display::Display() : oled(ANCHO, ALTO, &Wire, -1) {}

bool Display::iniciar(short sda, short scl) {
  Wire.begin(sda, scl);
  iniciada = oled.begin(DIRECCION_I2C, true);
  if (iniciada) {
    redibujar();
  }
  return iniciada;
}

// Dibuja el historial de comandos, un renglón debajo del otro.
void Display::dibujarBuffer() {
  for (int i = 0; i < CANT_HISTORIAL; i++) {
    oled.setCursor(0, Y_HISTORIAL + i * ALTO_RENGLON);
    oled.print(ultimos[i]);
  }
}

// Vuelve a dibujar la pantalla completa: mensaje arriba, separador e historial.
void Display::redibujar() {
  if (!iniciada) return;

  // Limpiar todo evita restos cuando el siguiente comando es mas corto.
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
  // Hasta llamar a display() los cambios quedan solo en memoria.
  oled.display();
}

void Display::mostrar(const String &linea1, const String &linea2) {
  lineaSuperior1 = limitarLinea(linea1);
  lineaSuperior2 = limitarLinea(linea2);
  redibujar();
}

// Corre el historial una posición hacia abajo (se pierde el más viejo)
// y pone el comando nuevo en la primera posición.
void Display::registrarComando(const String &cmd) {
  ultimos[2] = ultimos[1];
  ultimos[1] = ultimos[0];
  ultimos[0] = limitarLinea(cmd);
  redibujar();
}
