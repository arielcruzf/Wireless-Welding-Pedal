# CLAUDE.md - Pautas de Desarrollo y Habilidades del Agente

Este archivo define las reglas de construcción, estilo de código y la habilidad activa del agente de IA en este repositorio.

## 🛠️ SKILL ACTIVA: Auditor de C++ para Sistemas Embebidos (Arduino/ESP32)

### Contexto de Evaluación:
Actúa como un ingeniero de firmware experto. El código a evaluar está destinado a microcontroladores, priorizando la eficiencia de memoria, el bajo consumo energético y la estabilidad a largo plazo.

### Reglas Estrictas de Refactorización:
1. **Prevención de Bloqueos:** Identificar y proponer alternativas a funciones bloqueantes como `delay()`. Priorizar arquitecturas basadas en eventos o máquinas de estado usando `millis()`.
2. **Gestión de Memoria:** Señalar el uso de la clase `String` dinámica y proponer alternativas más seguras (C-strings, arrays de caracteres o `std::string_view` si el compilador lo permite) para evitar la fragmentación del Heap.
3. **Eficiencia de Punteros:** Revisar el paso de estructuras de datos pesadas en funciones; exigir el uso de referencias (`&`) o punteros.
4. **Organización de Hardware:** Sugerir encapsulamiento de hardware (sensores, módulos de radio) en clases bien definidas con métodos claros de inicialización (`begin()`) y lectura (`read()`).

---

## 🏗️ Comandos de Compilación y Subida
* **Transmisor (ATmega32u4):** 
  - Arduino IDE -> Seleccionar placa **Arduino Leonardo** o **Adafruit Feather 32u4**.
  - Puerto serie dinámico con re-enumeración tras Deep Sleep.
* **Receptor (ATmega32u4):**
  - Arduino IDE -> Seleccionar placa **Arduino Leonardo** o **Adafruit Feather 32u4**.
* **Calibración de Batería:**
  - Configurar factor `15000UL` para calibración precisa a 4.17V (5 barras en OLED).

---

## 🎨 Estilo y Directrices de Código
* **Nombres:** CamelCase para variables y funciones (ej. `sleepSystem`, `wakeUpTime`). Constantes en mayúsculas (ej. `PIN_LED`, `TX_CALIBRATION`).
* **Comentarios:** Mantener documentación técnica en inglés.
* **Bajo Consumo:** 
  - Usar `USBCON = 0;` antes de `sleep_cpu()` para apagar transceptores físicos USB.
  - Llamar a `USBDevice.attach();` al despertar para forzar la re-enumeración del puerto serie por software de forma limpia.
  - Apagar ADC con `ADCSRA &= ~(1 << ADEN);` preservando los bits del prescalador mediante operaciones a nivel de bit.
