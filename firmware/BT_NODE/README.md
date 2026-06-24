# BT_NODE

Firmware del nodo ESP32 clásico dedicado a Bluetooth.

## ID

`0x04`

## Modos

- `0`: HID_BLE_PC
- `1`: SPP_MODEM
- `2`: POWER_SAVE

## Funciones

- HID BLE para teclado y mouse.
- SPP clásico en modo servidor y cliente.
- Reinicio seguro de stack Bluetooth al cambiar de modo.
