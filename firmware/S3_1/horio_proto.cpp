#include "horio_proto.h"

uint16_t crc16_ccitt_false(const uint8_t* data, size_t len) {
  uint16_t crc = 0xFFFF;
  for (size_t i = 0; i < len; i++) {
    crc ^= (uint16_t)data[i] << 8;
    for (int b = 0; b < 8; b++) {
      if (crc & 0x8000) crc = (crc << 1) ^ 0x1021;
      else crc <<= 1;
    }
  }
  return crc;
}

void put_le16(uint8_t* p, uint16_t v){ p[0] = (uint8_t)(v & 0xFF); p[1] = (uint8_t)(v >> 8); }
uint16_t get_le16(const uint8_t* p){ return (uint16_t)p[0] | ((uint16_t)p[1] << 8); }

void send_frame(HardwareSerial& U, uint8_t src, uint8_t dst, MsgType type, uint8_t flags,
                uint16_t msg_id, uint16_t cmd, const uint8_t* data, uint16_t len) {
  uint8_t hdr[11];
  hdr[0]=VER; hdr[1]=src; hdr[2]=dst; hdr[3]=(uint8_t)type; hdr[4]=flags;
  put_le16(&hdr[5], msg_id);
  put_le16(&hdr[7], cmd);
  put_le16(&hdr[9], len);

  // CRC = CRC(hdr) luego CRC(data) con el mismo algoritmo que usas
  uint16_t crc = crc16_ccitt_false(hdr, sizeof(hdr));
  for (uint16_t i = 0; i < len; i++) {
    crc ^= (uint16_t)data[i] << 8;
    for (int b = 0; b < 8; b++) {
      if (crc & 0x8000) crc = (crc << 1) ^ 0x1021;
      else crc <<= 1;
    }
  }

  U.write(SOF1); U.write(SOF2);
  U.write(hdr, sizeof(hdr));
  if (len) U.write(data, len);
  U.write((uint8_t)(crc & 0xFF));
  U.write((uint8_t)(crc >> 8));
}
