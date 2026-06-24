#pragma once
#include <Arduino.h>
#include <stdint.h>
#include "horio_proto.h"

struct RxParser {
  enum State { WAIT_SOF1, WAIT_SOF2, READ_HDR, READ_DATA, READ_CRC } st = WAIT_SOF1;
  uint8_t hdr[11]; uint16_t hdr_i=0;
  uint16_t data_len=0; uint8_t data[256]; uint16_t data_i=0;
  uint8_t crc_b[2]; uint8_t crc_i=0;
  uint8_t ver, src, dst, type, flags;
  uint16_t msg_id, cmd;
  void reset(){ st=WAIT_SOF1; hdr_i=0; data_len=0; data_i=0; crc_i=0; }
};

typedef void (*frame_handler_fn)(HardwareSerial& U,
                                 uint8_t src, uint8_t dst, uint8_t type,
                                 uint16_t cmd, uint16_t msg_id,
                                 const uint8_t* data, uint16_t len);

void rx_feed(RxParser& rx, HardwareSerial& U, frame_handler_fn handler);
