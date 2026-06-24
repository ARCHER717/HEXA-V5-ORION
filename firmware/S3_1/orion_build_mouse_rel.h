#pragma once
#include <stdint.h>
#include "orion_input_events.h"

#define ORION_MOUSE_REL_PAYLOAD_LEN 4
#define ORION_MOUSE_REL_DATA_LEN (ORION_OFF_PAYLOAD + ORION_MOUSE_REL_PAYLOAD_LEN)

static inline void orion_build_mouse_rel(uint8_t *data,
                                         uint8_t seq,
                                         uint8_t flags,
                                         int8_t dx,
                                         int8_t dy,
                                         int8_t wheel,
                                         uint8_t buttons)
{
  data[ORION_OFF_EVT_ID]  = EVT_INPUT_MOUSE_REL;
  data[ORION_OFF_TARGET]  = TGT_PC_HID;
  data[ORION_OFF_SEQ]     = seq;
  data[ORION_OFF_FLAGS]   = flags;

  data[ORION_OFF_PAYLOAD + 0] = (uint8_t)dx;
  data[ORION_OFF_PAYLOAD + 1] = (uint8_t)dy;
  data[ORION_OFF_PAYLOAD + 2] = (uint8_t)wheel;
  data[ORION_OFF_PAYLOAD + 3] = buttons;
}
