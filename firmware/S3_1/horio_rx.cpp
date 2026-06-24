#include "horio_rx.h"

void rx_feed(RxParser& rx, HardwareSerial& U, frame_handler_fn handler) {
  while (U.available()) {
    uint8_t b = (uint8_t)U.read();
    switch (rx.st) {
      case RxParser::WAIT_SOF1:
        if (b==SOF1) rx.st=RxParser::WAIT_SOF2;
        break;

      case RxParser::WAIT_SOF2:
        if (b==SOF2){ rx.st=RxParser::READ_HDR; rx.hdr_i=0; }
        else rx.st=RxParser::WAIT_SOF1;
        break;

      case RxParser::READ_HDR:
        rx.hdr[rx.hdr_i++] = b;
        if (rx.hdr_i == sizeof(rx.hdr)) {
          rx.ver=rx.hdr[0]; rx.src=rx.hdr[1]; rx.dst=rx.hdr[2]; rx.type=rx.hdr[3]; rx.flags=rx.hdr[4];
          rx.msg_id=get_le16(&rx.hdr[5]); rx.cmd=get_le16(&rx.hdr[7]); rx.data_len=get_le16(&rx.hdr[9]);
          if (rx.ver!=VER || rx.data_len>sizeof(rx.data)) { rx.reset(); break; }
          rx.data_i=0; rx.crc_i=0;
          rx.st = (rx.data_len==0) ? RxParser::READ_CRC : RxParser::READ_DATA;
        }
        break;

      case RxParser::READ_DATA:
        rx.data[rx.data_i++] = b;
        if (rx.data_i == rx.data_len) { rx.st=RxParser::READ_CRC; rx.crc_i=0; }
        break;

      case RxParser::READ_CRC:
        rx.crc_b[rx.crc_i++] = b;
        if (rx.crc_i == 2) {
          uint16_t rx_crc = (uint16_t)rx.crc_b[0] | ((uint16_t)rx.crc_b[1] << 8);

          uint16_t crc = crc16_ccitt_false(rx.hdr, sizeof(rx.hdr));
          for (uint16_t i=0;i<rx.data_len;i++){
            crc ^= (uint16_t)rx.data[i] << 8;
            for (int bb=0;bb<8;bb++){
              if (crc & 0x8000) crc = (crc << 1) ^ 0x1021;
              else crc <<= 1;
            }
          }

          if (crc == rx_crc && handler) {
            handler(U, rx.src, rx.dst, rx.type, rx.cmd, rx.msg_id, rx.data, rx.data_len);
          }
          rx.reset();
        }
        break;
    }
  }
}
