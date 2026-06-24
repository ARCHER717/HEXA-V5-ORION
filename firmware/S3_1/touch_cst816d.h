#pragma once
#include <Arduino.h>
#include <Wire.h>
#include <stdint.h>

struct TouchPoint {
  bool down;
  uint16_t x;
  uint16_t y;
  uint8_t gesture;
};

bool cst816d_init(uint8_t sda_pin,
                  uint8_t scl_pin,
                  uint8_t rst_pin,
                  uint8_t int_pin,
                  uint8_t i2c_addr = 0x15);

bool cst816d_read(TouchPoint& p);
