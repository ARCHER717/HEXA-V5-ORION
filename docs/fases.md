# Fases del proyecto HEXA V5 ORION

## Fase 1 — Base de hardware

Definición de nodos, pines, alimentación y conexiones principales.

## Fase 2 — Transporte HORIO

Implementación de comunicación UART multinodo, tramas, CRC y direccionamiento.

## Fase 3 — Eventos ORION

Definición de eventos de entrada, comandos y rutas.

## Fase 4 — UI P4

Integración de LVGL/SquareLine con callbacks externos.

## Fase 5 — Nodo S3_1

Trackpad táctil, pantalla redonda y envío de eventos.

## Fase 6 — Nodo BT HID

Mouse y teclado HID BLE desde eventos ORION.

## Fase 7 — BT Mode Manager

Modos HID_BLE_PC, SPP_MODEM y POWER_SAVE.

## Fase 7.3.x — Perfiles Bluetooth

Gestión de perfiles Bluetooth clásicos desde el P4 y SD por SDIO.

## Fases futuras

- Integración LoRa.
- Integración ESP-NOW.
- Mejoras de UI.
- Sistema de perfiles globales.
- Pruebas de robustez y recuperación.
