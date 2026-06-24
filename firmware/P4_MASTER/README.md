# P4_MASTER

Firmware del nodo maestro ESP32-P4.

## Funciones

- Interfaz principal LVGL/SquareLine.
- Router de eventos ORION.
- Comunicación HORIO con nodos secundarios.
- Gestión de SD por SDIO.

## Regla

No modificar directamente archivos generados `ui_*`. Usar callbacks externos.
