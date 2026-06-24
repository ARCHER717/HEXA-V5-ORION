#include "touch_cst816d.h"

static uint8_t  g_addr = 0x15;
static uint16_t g_w = 240, g_h = 240;

// Pines
static uint8_t g_rst = 255;
static uint8_t g_int = 255;

// Ajustes de orientación (CAMBIA SOLO ESTO SI HACE FALTA)
static bool SWAP_XY = false;
static bool INV_X   = true;
static bool INV_Y   = true;

static inline void transform_xy(uint16_t &x, uint16_t &y)
{
  // Escala si viniera en 12 bits
  if (x > g_w - 1 || y > g_h - 1) {
    x = (uint16_t)((uint32_t)x * g_w / 4095);
    y = (uint16_t)((uint32_t)y * g_h / 4095);
  }

  if (SWAP_XY) { uint16_t t = x; x = y; y = t; }
  if (INV_X)   x = (g_w - 1) - x;
  if (INV_Y)   y = (g_h - 1) - y;
}

static bool i2c_read_reg(uint8_t reg, uint8_t* out, uint8_t n)
{
  Wire.beginTransmission(g_addr);
  Wire.write(reg);
  if (Wire.endTransmission(false) != 0) return false;
  if (Wire.requestFrom((int)g_addr, (int)n) != n) return false;
  for (uint8_t i = 0; i < n; i++) out[i] = Wire.read();
  return true;
}

bool cst816d_init(uint8_t sda_pin,
                  uint8_t scl_pin,
                  uint8_t rst_pin,
                  uint8_t int_pin,
                  uint8_t i2c_addr)
{
  g_addr = i2c_addr;
  g_rst  = rst_pin;
  g_int  = int_pin;

  pinMode(g_rst, OUTPUT);
  digitalWrite(g_rst, LOW);  delay(10);
  digitalWrite(g_rst, HIGH); delay(50);

  pinMode(g_int, INPUT_PULLUP);

  Wire.begin(sda_pin, scl_pin, 400000);

  Wire.beginTransmission(g_addr);
  return (Wire.endTransmission() == 0);
}

bool cst816d_read(TouchPoint& p)
{
  p.down    = false;
  p.x       = 0;
  p.y       = 0;
  p.gesture = 0;

  // Gesture ID (0x01)
  uint8_t gest = 0;
  if (!i2c_read_reg(0x01, &gest, 1)) return false;
  p.gesture = gest;

  // Finger status (0x02)
  uint8_t finger = 0;
  if (!i2c_read_reg(0x02, &finger, 1)) return false;

  if ((finger & 0x0F) == 0) {
    p.down = false;
    return true;
  }

  // Coordinates (0x03..0x06)
  uint8_t b[4] = {0};
  if (!i2c_read_reg(0x03, b, 4)) return false;

  uint16_t x = ((b[0] & 0x0F) << 8) | b[1];
  uint16_t y = ((b[2] & 0x0F) << 8) | b[3];

  transform_xy(x, y);

  p.down = true;
  p.x    = x;
  p.y    = y;
  return true;
}
