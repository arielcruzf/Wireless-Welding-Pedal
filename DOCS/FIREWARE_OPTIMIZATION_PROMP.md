# CONTEXTO DEL PROYECTO
Actúa como un Senior Embedded Systems Engineer. Estoy desarrollando un sistema de telemetría y control inalámbrico de baja latencia utilizando tecnología LoRa (433MHz) basado en la MCU ATmega32u4. 

El sistema consta de dos módulos: un TRANSMISOR (un pedal mecánico a batería) y un RECEPTOR (interfaz conectada a la máquina, alimentada externamente).

## HARDWARE DEL TRANSMISOR (TX)
- MCU: LoRa32u4 RA-02 433MHz (ATmega32u4 + SX1278)
- Energía: Batería LiPo 3.7V 1500mAh
- Sensor Analógico/Distancia: VL53L4CD ToF (I2C) para lectura de recorrido.
- Activador: Interruptor de límite KW12 (NO/NC).
- Interfaz de usuario: Botón LED M16.

## HARDWARE DEL RECEPTOR (RX)
- MCU: LoRa32u4 RA-02 433MHz (ATmega32u4 + SX1278)
- Energía: Batería LiPo 3.7V 1500mAh
- Display: 0.96" OLED 128x64 SSD1306 (I2C)
- Control de Potencia: Módulo MOSFET Aislado LR7843 (Control PWM)
- Salida Analógica: Convertidor PWM-a-DAC (Salida 0-10V)
- Interfaz de usuario: Botón LED M16.

# OBJETIVO
Necesito diseñar la arquitectura del firmware en Embedded C/C++ garantizando máxima fiabilidad, cero bloqueos, seguridad ante fallos (failsafe) y un consumo de batería mínimo en el TX.

# REQUERIMIENTOS DE OPTIMIZACIÓN (APLICACIÓN DE SKILLS)

1. Finite State Machines (FSM) & Arquitectura No Bloqueante:
Diseña el flujo del programa sin usar la función `delay()`. Utiliza `millis()` y máquinas de estados para manejar la lectura del sensor ToF I2C, la actualización de la pantalla OLED y las rutinas de transmisión/recepción simultáneamente.

2. Ultra-Low Power Optimization & Hardware Interrupts (TX):
El transmisor funcionará con batería. Diseña la estrategia para que la MCU esté en modo *Deep Sleep*. Utiliza el interruptor KW12 conectado a un pin de interrupción de hardware (Hardware Interrupt) para despertar el sistema instantáneamente solo cuando se presiona el pedal, y volver a dormir tras un periodo de inactividad.

3. Payload Optimization & LoRa Telemetry:
Diseña una estructura de datos (`struct`) ultra-ligera para enviar el estado del interruptor y el valor del ToF (mapeado de 0 a 255). El objetivo es que el paquete de datos (Payload) sea lo más pequeño posible para reducir el "Time-on-Air" del módulo SX1278, minimizando la latencia (crítico para control en tiempo real).

4. Signal Processing & Failsafe Integration:
Propón un filtro por software simple (ej. filtro de media móvil o paso bajo) para suavizar las lecturas del sensor VL53L4CD antes de enviarlas. Además, define una lógica de *Watchdog* o *Timeout* en el Receptor (RX): si se pierde la conexión LoRa durante más de 'X' milisegundos, el MOSFET de control y la salida 0-10V deben caer a 0 inmediatamente por seguridad.

# ENTREGABLES ESPERADOS
1. Estructura de la Máquina de Estados (diagrama lógico o lista de estados) para TX y RX.
2. Definición del `struct` optimizado en C++ para el Payload LoRa.
3. Un esquema de código (`pseudo-código` o esqueleto en C++) para la configuración del *Deep Sleep* y la Interrupción de Hardware en el ATmega32u4.
