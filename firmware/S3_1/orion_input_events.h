#pragma once
#include <stdint.h>

#define TGT_PC_HID   0x10

#define ORION_OFF_EVT_ID   0
#define ORION_OFF_TARGET   1
#define ORION_OFF_SEQ      2
#define ORION_OFF_FLAGS    3
#define ORION_OFF_PAYLOAD  4

#define EVT_INPUT_KEY        0x10
#define EVT_INPUT_MOUSE_REL  0x11
#define EVT_INPUT_MOUSE_ABS  0x12
#define EVT_INPUT_JOYSTICK   0x13

#define ORION_FLAG_ACK_REQ   (1u << 0)
#define ORION_FLAG_URGENT    (1u << 1)

#define MOUSE_BTN_LEFT   (1u << 0)
#define MOUSE_BTN_RIGHT  (1u << 1)
#define MOUSE_BTN_MIDDLE (1u << 2)
