#include "orion_mouse_trackpad.h"
#include "horio_proto.h"
#include "orion_build_mouse_rel.h"
#include <math.h>

// ====================== Estado interno ======================
static OrionTrackpadCfg g_cfg;
static uint8_t  g_seq_mouse = 0;

// Estado del motor REL
static bool     g_rel_down = false;
static int16_t  g_last_x = 0;
static int16_t  g_last_y = 0;
static uint32_t g_last_send_ms = 0;

// Botones HID
static uint8_t  g_btn_state = 0; // bit0=left, bit1=right

// Touch para long-press
static bool     g_tp_down = false;
static uint32_t g_down_ms = 0;
static int16_t  g_x0 = 0, g_y0 = 0;

// Click izquierdo por tiempo (mantener)
static bool     g_left_latch = false;
static const uint16_t LONGPRESS_MS = 450;      // ajusta: 350..650
static const uint8_t  LONGPRESS_MOVE_PX = 6;   // tolerancia de movimiento mientras mantienes

// ----- 7.1.1 tuning: suavizado + aceleración + rate adaptativo -----
static float    g_vx = 0.0f, g_vy = 0.0f;       // velocidad filtrada
static uint32_t g_last_move_ms = 0;

// 0.25..0.50 (más bajo = más suave / más alto = más reactivo)
static const float LP_ALPHA = 0.35f;

// periodos de envío (ms). Menor = más frames/seg
static const uint16_t STILL_HZ_MS  = 25; // casi quieto
static const uint16_t IDLE_HZ_MS   = 18; // movimiento normal
static const uint16_t ACTIVE_HZ_MS = 8;  // movimiento rápido
// ============================================================

static int8_t clamp_i8(int32_t v){
  if (v > 127) return 127;
  if (v < -127) return -127;
  return (int8_t)v;
}

// Enviar ORION mouse REL
static void send_orion_mouse_rel(HardwareSerial& U,
                                 int8_t dx, int8_t dy,
                                 int8_t wh,
                                 uint8_t buttons)
{
  uint8_t data[ORION_MOUSE_REL_DATA_LEN];
  orion_build_mouse_rel(data, g_seq_mouse++, 0, dx, dy, wh, buttons);
  send_frame(U, ID_SELF, ID_P4, EVT, 0x00, 0,
             CMD_ORION_EVENT, data, ORION_MOUSE_REL_DATA_LEN);
}

// Motor de movimiento REL (deadzone + smoothing + accel + adaptive rate)
static void touch_to_orion_mouse_rel(HardwareSerial& U,
                                     int16_t x, int16_t y,
                                     bool down,
                                     uint32_t now_ms)
{
  // DOWN edge
  if (down && !g_rel_down) {
    g_rel_down = true;
    g_last_x = x;
    g_last_y = y;
    g_last_send_ms = now_ms;
    g_last_move_ms = now_ms;
    g_vx = g_vy = 0.0f;
    return;
  }

  // UP edge
  if (!down && g_rel_down) {
    g_rel_down = false;
    g_vx = g_vy = 0.0f;
    return;
  }

  if (!g_rel_down) return;

  int16_t raw_dx = x - g_last_x;
  int16_t raw_dy = y - g_last_y;

  g_last_x = x;
  g_last_y = y;

  int16_t mag = abs(raw_dx) + abs(raw_dy);

  // Deadzone
  if (mag < g_cfg.deadzone_px) {
    // decay suave para “apagar” el jitter
    g_vx *= 0.85f;
    g_vy *= 0.85f;
    return;
  }

  // -------- Filtro low-pass (suaviza temblor) --------
  g_vx = (1.0f - LP_ALPHA) * g_vx + LP_ALPHA * (float)raw_dx;
  g_vy = (1.0f - LP_ALPHA) * g_vy + LP_ALPHA * (float)raw_dy;

  // -------- Aceleración tipo trackpad --------
  float base_gain = (float)g_cfg.gain_q8 / 256.0f;

  float accel = 1.0f;
  if (mag > 6)  accel = 1.25f;
  if (mag > 12) accel = 1.60f;
  if (mag > 20) accel = 2.00f;
  if (mag > 30) accel = 2.60f;

  float fx = g_vx * base_gain * accel;
  float fy = g_vy * base_gain * accel;

  // -------- Rate adaptativo --------
  uint16_t period_ms = STILL_HZ_MS;
  if (mag > 10) period_ms = IDLE_HZ_MS;
  if (mag > 22) period_ms = ACTIVE_HZ_MS;

  if ((now_ms - g_last_send_ms) < period_ms) return;

  // Round + clamp
  int8_t dx = clamp_i8((int32_t)lroundf(fx));
  int8_t dy = clamp_i8((int32_t)lroundf(fy));

  if (dx == 0 && dy == 0) return;

  send_orion_mouse_rel(U, dx, dy, 0, g_btn_state);
  g_last_send_ms = now_ms;
  g_last_move_ms = now_ms;
}

// ====================== API ======================

void orion_mouse_trackpad_init(const OrionTrackpadCfg& cfg){
  g_cfg = cfg;

  g_seq_mouse = 0;
  g_rel_down = false;
  g_tp_down = false;
  g_last_x = g_last_y = 0;
  g_last_send_ms = 0;

  g_btn_state = 0;

  g_down_ms = 0;
  g_x0 = g_y0 = 0;
  g_left_latch = false;

  g_vx = g_vy = 0.0f;
  g_last_move_ms = 0;
}

void orion_mouse_trackpad_set_left_click(bool pressed){
  if (pressed) g_btn_state |=  (1u << 0);
  else         g_btn_state &= ~(1u << 0);
}

void orion_mouse_trackpad_set_right_click(bool pressed){
  if (pressed) g_btn_state |=  (1u << 1);
  else         g_btn_state &= ~(1u << 1);
}

bool orion_mouse_trackpad_on_touch(HardwareSerial& U, int16_t x, int16_t y, bool down){
  touch_to_orion_mouse_rel(U, x, y, down, millis());
  return true;
}

// SOLO: mover + long-press LEFT CLICK (mantener)
void orion_mouse_trackpad_on_touch_ex(HardwareSerial& U,
                                      int16_t x, int16_t y,
                                      bool down,
                                      uint8_t gesture)
{
  (void)gesture; // ignoramos taps/gestos del chip

  const uint32_t now = millis();

  // Movimiento
  touch_to_orion_mouse_rel(U, x, y, down, now);

  // Edges del dedo
  bool down_edge = (down && !g_tp_down);
  bool up_edge   = (!down && g_tp_down);
  g_tp_down = down;

  if (down_edge) {
    g_down_ms = now;
    g_x0 = x;
    g_y0 = y;
    g_left_latch = false;
    return;
  }

  if (down) {
    if (!g_left_latch) {
      uint32_t held = now - g_down_ms;
      int move = abs(x - g_x0) + abs(y - g_y0);

      // Long press => LEFT DOWN
      if (held >= LONGPRESS_MS && move <= LONGPRESS_MOVE_PX) {
        g_left_latch = true;
        orion_mouse_trackpad_set_left_click(true);
        send_orion_mouse_rel(U, 0, 0, 0, g_btn_state);
      }
    }
    return;
  }

  if (up_edge) {
    // Soltar LEFT si estaba presionado
    if (g_left_latch) {
      g_left_latch = false;
      orion_mouse_trackpad_set_left_click(false);
      send_orion_mouse_rel(U, 0, 0, 0, g_btn_state);
    }
    return;
  }
}

// Debug opcional
void orion_mouse_trackpad_tick(HardwareSerial& U){
  while (Serial.available()){
    char c = Serial.read();
    if (c=='\r' || c=='\n') continue;

    int dx=0, dy=0;
    if (c=='a') dx = -8;
    if (c=='d') dx =  8;
    if (c=='w') dy = -8;
    if (c=='s') dy =  8;

    if (dx || dy)
      send_orion_mouse_rel(U, clamp_i8(dx), clamp_i8(dy), 0, g_btn_state);
  }
}
