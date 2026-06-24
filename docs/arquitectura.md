# Arquitectura general

## Nodo maestro

- **P4_MASTER**
- Hardware: ESP32-P4 TAB5
- ID: `0x01`
- Funciones:
  - UI principal LVGL/SquareLine.
  - Router ORION.
  - Coordinación de nodos.
  - SD por SDIO.

## Nodos secundarios

### S3_1

- Hardware: ESP32-S3 con pantalla redonda.
- ID: `0x02`.
- Función: interfaz táctil, trackpad, panel de control y comunicaciones.

### S3_2

- Hardware: ESP32-S3 con pantalla redonda.
- ID: `0x03`.
- Función: panel secundario y expansión.

### BT_NODE

- Hardware: ESP32 clásico WROOM.
- ID: `0x04`.
- Función: Bluetooth exclusivo.
- Modos:
  - `0`: HID_BLE_PC.
  - `1`: SPP_MODEM.
  - `2`: POWER_SAVE.

## Transporte interno

- Protocolo base: HORIO.
- Capa de eventos: ORION v0.1.
- Baudrate UART: `460800`.
- CRC: CRC16-CCITT-FALSE.
