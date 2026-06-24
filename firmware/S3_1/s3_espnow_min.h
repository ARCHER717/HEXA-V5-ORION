#pragma once
#include <Arduino.h>
#include <esp_now.h>

struct S3EspNowTestPacket {
  uint8_t  magic;       // 0xE8
  uint8_t  version;     // 0x01
  uint8_t  msg_type;    // 0x00 = test
  uint8_t  seq;
  uint32_t uptime_ms;
  char     text[32];
};

bool s3_espnow_init(uint8_t wifi_channel);
void s3_espnow_deinit();

bool s3_espnow_set_peer(const uint8_t mac[6], bool encrypt);
bool s3_espnow_send_test();

void s3_espnow_loop();

bool s3_espnow_is_ready();
uint8_t s3_espnow_get_channel();