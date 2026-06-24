#include "s3_espnow_min.h"
#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>

extern void hexa_send_frame(uint8_t dst, uint8_t type, uint16_t cmd,
                            const uint8_t* payload, uint16_t len, uint16_t msg_id);

#ifndef ID_P4
#define ID_P4 0x01
#endif

#ifndef ID_S3_1
#define ID_S3_1 0x02
#endif

#ifndef CMD_TEXT_SEND
#define CMD_TEXT_SEND 0x1000
#endif

#ifndef CMD_S3_ESPNOW_TX
#define CMD_S3_ESPNOW_TX 0x3001
#endif
#ifndef CMD_S3_ESPNOW_RX
#define CMD_S3_ESPNOW_RX 0x3002
#endif
#ifndef CMD_S3_ESPNOW_STATUS
#define CMD_S3_ESPNOW_STATUS 0x3003
#endif

static bool g_esnow_ready = false;

// Broadcast peer
static uint8_t g_bcast[6] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};

static void p4_log_text(const char* s){
  if(!s) return;
  hexa_send_frame(ID_P4, 0x02, CMD_TEXT_SEND, (const uint8_t*)s, (uint16_t)strlen(s), 0);
}

static void send_status_to_p4(const char* s){
  if(!s) return;
  hexa_send_frame(ID_P4, 0x03, CMD_S3_ESPNOW_STATUS, (const uint8_t*)s, (uint16_t)strlen(s), 0);
}

#if ESP_ARDUINO_VERSION_MAJOR >= 3
static void onDataSent(const wifi_tx_info_t *tx_info, esp_now_send_status_t status){
  (void)tx_info;
  send_status_to_p4(status == ESP_NOW_SEND_SUCCESS ? "ESP-NOW TX OK" : "ESP-NOW TX FAIL");
}
#else
static void onDataSent(const uint8_t *mac_addr, esp_now_send_status_t status){
  (void)mac_addr;
  send_status_to_p4(status == ESP_NOW_SEND_SUCCESS ? "ESP-NOW TX OK" : "ESP-NOW TX FAIL");
}
#endif

#if ESP_ARDUINO_VERSION_MAJOR >= 3
static void onDataRecv(const esp_now_recv_info_t *info, const uint8_t *data, int len){
  (void)info;
  if(!data || len <= 0) return;
  if(len > 220) len = 220;

  uint8_t buf[240];
  buf[0] = (uint8_t)len;
  memcpy(&buf[1], data, len);
  hexa_send_frame(ID_P4, 0x03, CMD_S3_ESPNOW_RX, buf, (uint16_t)(len + 1), 0);
}
#else
static void onDataRecv(const uint8_t *mac, const uint8_t *data, int len){
  (void)mac;
  if(!data || len <= 0) return;
  if(len > 220) len = 220;

  uint8_t buf[240];
  buf[0] = (uint8_t)len;
  memcpy(&buf[1], data, len);
  hexa_send_frame(ID_P4, 0x03, CMD_S3_ESPNOW_RX, buf, (uint16_t)(len + 1), 0);
}
#endif

bool s3_espnow_min_init(){
  if(g_esnow_ready) return true;

  WiFi.mode(WIFI_STA);
  WiFi.disconnect(false, false);
  delay(50);

  if(esp_now_init() != ESP_OK){
    p4_log_text("[S3_1 ESPNOW] init FAIL");
    return false;
  }

  esp_now_register_send_cb(onDataSent);
  esp_now_register_recv_cb(onDataRecv);

  esp_now_peer_info_t peer{};
  memcpy(peer.peer_addr, g_bcast, 6);
  peer.channel = 0;
  peer.encrypt = false;

  if(!esp_now_is_peer_exist(g_bcast)){
    if(esp_now_add_peer(&peer) != ESP_OK){
      p4_log_text("[S3_1 ESPNOW] add peer FAIL");
      // ESP-NOW still can receive; but TX broadcast may fail.
    }
  }

  g_esnow_ready = true;
  p4_log_text("[S3_1 ESPNOW] ready");
  return true;
}

bool s3_espnow_min_send_broadcast(const uint8_t* data, uint8_t len){
  if(!data || len == 0) return false;
  if(!g_esnow_ready){
    if(!s3_espnow_min_init()) return false;
  }
  esp_err_t e = esp_now_send(g_bcast, data, len);
  return e == ESP_OK;
}

void s3_espnow_min_handle_p4_tx(const uint8_t* payload, uint16_t len){
  if(!payload || len == 0) return;
  if(len > 220) len = 220;

  bool ok = s3_espnow_min_send_broadcast(payload, (uint8_t)len);
  send_status_to_p4(ok ? "S3_1 ESP-NOW send queued" : "S3_1 ESP-NOW send error");
}
