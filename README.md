# TPA_IOT

Sistema IoT desarrollado con ESP32 y PlatformIO para monitorear variables ambientales y controlar actuadores mediante Wi‑Fi, pantalla OLED y Telegram.

## Descripción

Este proyecto integra varios elementos de la práctica de laboratorio en una misma solución:

- Lectura de temperatura y humedad con sensor DHT22
- Medición de voltaje mediante potenciómetro
- Control de LEDs
- Visualización de datos en pantalla OLED SH1106
- Conexión a red Wi‑Fi
- Envío de datos a ThingSpeak
- Interacción con un bot de Telegram para consultar y controlar el sistema

La lógica principal se encuentra en `TPI_grupo_5/src/main.cpp`, y cada periférico está encapsulado en librerías dentro de `TPI_grupo_5/lib/` para mantener el código modular y reutilizable.

## Objetivo

Monitorear condiciones ambientales en tiempo real, visualizar información localmente en la pantalla OLED y permitir control remoto a través de Telegram, además de registrar datos en la nube para análisis posterior.

## Componentes principales

- ESP32 DevKit V1
- Sensor DHT22
- Potenciómetro
- LEDs (verde y azul)
- Pantalla OLED 128x64 con controlador SH1106
- Wi‑Fi
- Bot de Telegram
- ThingSpeak

## Estructura del proyecto

```text
TPA_IOT/
├── README.md
├── TPI_grupo_5/
│   ├── include/
│   │   └── secrets.example.h    # plantilla para crear secrets.h localmente
│   ├── lib/
│   │   ├── BotTelegram/
│   │   ├── ClienteThingSpeak/
│   │   ├── ControlLed/
│   │   ├── Display/
│   │   ├── Potenciometro/
│   │   ├── SensorDHT/
│   │   └── WiFiConnection/
│   ├── src/
│   │   └── main.cpp
│   ├── test/
│   ├── platformio.ini
│   └── wokwi.toml
└── ...
```

## Requisitos

- PlatformIO IDE o VS Code con extensión PlatformIO
- ESP32 compatible
- Paquetes/librerías declaradas en `TPI_grupo_5/platformio.ini`
- Credenciales Wi‑Fi
- Token de Telegram
- Write API Key de ThingSpeak

## Configuración

> Importante: `include/secrets.h` no viene en el repositorio. Es un archivo local que vos tenés que crear para guardar tus credenciales. El proyecto lo incluye desde `main.cpp`, y si no existe, el compilado falla.

1. Abrir la carpeta `TPI_grupo_5` como proyecto de PlatformIO.
2. Crear el archivo de credenciales a partir de la plantilla:

```bash
cp include/secrets.example.h include/secrets.h
```

3. Completar los valores en `include/secrets.h`:

```cpp
#define SECRET_SSID "TU_SSID"
#define SECRET_PASS "TU_PASSWORD"
#define SECRET_TELEGRAM_TOKEN "TU_TOKEN_DE_TELEGRAM"
#define SECRET_TS_API_KEY "TU_WRITE_API_KEY"
#define SECRET_TS_CHANNEL 123456
```

> `secrets.h` no debe subirse a GitHub. Se usa para guardar credenciales locales del proyecto.

4. Compilar y cargar el firmware en la placa ESP32.

## Cómo funciona

El programa principal realiza lo siguiente:

- Inicializa sensores, LEDs, OLED y conexión Wi‑Fi
- Conecta la placa a la red Wi‑Fi
- Inicia el bot de Telegram si la conexión es correcta
- Lee temperatura, humedad y voltaje periódicamente
- Muestra el estado en la consola serial y en la OLED
- Envia telemetría a ThingSpeak cada cierto intervalo
- Permite controlar LEDs, ajustar el umbral y consultar estados desde Telegram

## Comandos de Telegram

El bot acepta estos comandos:

- `/start` — Muestra el menú de comandos
- `/estado` — Muestra temperatura, humedad, voltaje y el estado de los LEDs
- `/umbral <valor>` — Ajusta el umbral de activación del LED azul
- `/autoon` — Activa el modo automático
- `/autooff` — Desactiva el modo automático
- `/thingup` — Envía datos manualmente a ThingSpeak
- `/displayled` — Muestra el estado de los LEDs en la pantalla OLED
- `/displaypot` — Muestra el voltaje del potenciómetro en la OLED
- `/displaydht` — Muestra temperatura y humedad en la OLED

## Pines utilizados

Los pines configurados en `src/main.cpp` son:

- DHT22: GPIO 33
- Potenciómetro: GPIO 32
- LED verde: GPIO 23
- LED azul: GPIO 2
- OLED (I2C): SDA = GPIO 21, SCL = GPIO 22

## Consideraciones importantes

- Se utiliza `millis()` para evitar bloqueos en el programa y permitir atender varias tareas simultáneamente.
- La subida a ThingSpeak está restringida por el intervalo mínimo de la plataforma.
- Si la conexión a Wi‑Fi falla, el sistema reintenta automáticamente.
- El bot de Telegram solo funciona si hay conexión a Internet y la API del token está configurada correctamente.

## Uso en Wokwi

El proyecto también incluye archivo `wokwi.toml`, por lo que puede simularse en Wokwi usando una red de prueba como `Wokwi-GUEST` o ajustando los valores de `secrets.h` según el entorno.

## Licencia

Este proyecto se entrega para uso académico y de aprendizaje dentro del contexto de la materia TPA_IOT.

