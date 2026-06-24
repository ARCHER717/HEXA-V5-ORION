#pragma once
#include <Arduino.h>
#include <stdint.h>

// Config del trackpad (ajustable)
struct OrionTrackpadCfg {
  uint8_t  deadzone_px   = 2;    // umbral mínimo (px)
  uint16_t rate_limit_ms = 10;   // 100 Hz
  uint16_t gain_q8       = 384;  // Q8.8: 256=1.0x, 384=1.5x
};

void orion_mouse_trackpad_init(const OrionTrackpadCfg& cfg = OrionTrackpadCfg());

// API básica (por si la sigues usando)
bool orion_mouse_trackpad_on_touch(HardwareSerial& U, int16_t x, int16_t y, bool down);

// API extendida “trackpad laptop-like” (SIN scroll)
// gesture: lo recibimos pero NO lo usamos para scroll.
void orion_mouse_trackpad_on_touch_ex(HardwareSerial& U,
                                      int16_t x, int16_t y,
                                      bool down,
                                      uint8_t gesture);

// Botones (por si luego usas UI)
void orion_mouse_trackpad_set_left_click(bool pressed);
void orion_mouse_trackpad_set_right_click(bool pressed);

// Debug opcional (WASD por USB Serial)
void orion_mouse_trackpad_tick(HardwareSerial& U);
