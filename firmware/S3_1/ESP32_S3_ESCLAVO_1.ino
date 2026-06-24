#include <Arduino.h>
#include "horio_proto.h"
#include "horio_rx.h"
#include "s3_services.h"
#include "orion_mouse_trackpad.h"
#include "touch_cst816d.h"
#include "s3_espnow_min.h"
// Touch pins (CST816D) según tu imagen (CONGELADOS)
#define PIN_TP_SDA  1
#define PIN_TP_SCL  2
#define PIN_TP_INT  9
#define PIN_TP_RST  38
#define TP_I2C_ADDR 0x15

static RxParser rx;

// Handler (tu lógica RX original)
static void handle_frame(HardwareSerial& U,
                         uint8_t src, uint8_t dst, uint8_t type,
                         uint16_t cmd, uint16_t msg_id,
                         const uint8_t* data, uint16_t len)
{
  if (dst != ID_SELF) return;

  if (type == REQ && cmd == CMD_PING) {
    uint32_t up = (uint32_t)millis();
    uint8_t payload[4] = {
      (uint8_t)up, (uint8_t)(up>>8), (uint8_t)(up>>16), (uint8_t)(up>>24)
    };
    send_frame(U, ID_SELF, ID_P4, RESP, 0x00, msg_id, CMD_PING, payload, 4);
    return;
  }

  if (type == REQ && cmd == CMD_TEXT_SEND) {
    Serial.print("S3(1) RX TEXT: ");
    for (uint16_t i=0; i<len; i++) Serial.write((char)data[i]);
    Serial.println();
    send_frame(U, ID_SELF, ID_P4, RESP, 0x00, msg_id, CMD_TEXT_ACK, nullptr, 0);
    return;
  }

  if (type == REQ && cmd == CMD_S3_ESPNOW_INIT) {
    uint8_t channel = 1;
    if (len >= 1) channel = data[0];
    if (channel < 1 || channel > 14) channel = 1;

    bool ok = s3_espnow_init(channel);

    uint8_t resp[2];
    resp[0] = ok ? 0x00 : 0x01;
    resp[1] = channel;

    Serial.printf("[RX][ESPNOW_INIT] ch=%u ok=%u\n", channel, ok ? 1 : 0);
    send_frame(U, ID_SELF, ID_P4, RESP, 0x00, msg_id, CMD_S3_ESPNOW_INIT, resp, sizeof(resp));
    return;
  }

  if (type == REQ && cmd == CMD_S3_ESPNOW_SET_PEER) {
    bool ok = false;

    if (len >= 7) {
      uint8_t mac6[6];
      memcpy(mac6, &data[0], 6);
      bool encrypt = (data[6] != 0);

      ok = s3_espnow_set_peer(mac6, encrypt);

      Serial.printf("[RX][ESPNOW_SET_PEER] mac=%02X:%02X:%02X:%02X:%02X:%02X enc=%u ok=%u\n",
                    mac6[0], mac6[1], mac6[2], mac6[3], mac6[4], mac6[5],
                    encrypt ? 1 : 0, ok ? 1 : 0);
    } else {
      Serial.printf("[RX][ESPNOW_SET_PEER] payload corto len=%u\n", len);
    }

    uint8_t resp[1];
    resp[0] = ok ? 0x00 : 0x01;
    send_frame(U, ID_SELF, ID_P4, RESP, 0x00, msg_id, CMD_S3_ESPNOW_SET_PEER, resp, sizeof(resp));
    return;
  }

  if (type == REQ && cmd == CMD_S3_ESPNOW_TEST_SEND) {
    bool ok = s3_espnow_send_test();

    uint8_t resp[1];
    resp[0] = ok ? 0x00 : 0x01;

    Serial.printf("[RX][ESPNOW_TEST_SEND] ok=%u\n", ok ? 1 : 0);
    send_frame(U, ID_SELF, ID_P4, RESP, 0x00, msg_id, CMD_S3_ESPNOW_TEST_SEND, resp, sizeof(resp));
    return;
  }
}

void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.println("S3(1) HORIO+ORION listo (FASE 7.1 TOUCH->ORION->HID)");

  // UART hacia P4
  Serial0.begin(BAUD, SERIAL_8N1, RX0_PIN, TX0_PIN);
  delay(100);

  // Init Trackpad ORION
OrionTrackpadCfg cfg;
cfg.deadzone_px = 2;
cfg.rate_limit_ms = 10;   // ya es adaptativo, esto no estorba
cfg.gain_q8 = 768;        // 512=2.0x (antes 384=1.5x)
orion_mouse_trackpad_init(cfg);

  // Init Touch CST816D (I2C)
  bool ok = cst816d_init(PIN_TP_SDA, PIN_TP_SCL, PIN_TP_RST, PIN_TP_INT, TP_I2C_ADDR);
  Serial.printf("Touch CST816D init: %s\n", ok ? "OK" : "FAIL");

  // Mensaje de vida (opcional)
  s3_send_orion_text(Serial0, "S3_1 -> TOUCH_READY\n");
}

void loop() {
  // 1) RX primero
  rx_feed(rx, Serial0, handle_frame);

  // 2) Heartbeat
  s3_send_heartbeat(Serial0);

  // 3) Touch -> ORION mouse REL
  static TouchPoint tp;
if (cst816d_read(tp)) {
  orion_mouse_trackpad_on_touch_ex(Serial0,(int16_t)tp.x,(int16_t)tp.y,tp.down,tp.gesture);
}

static uint32_t last_g = 0;
if (tp.gesture != 0 && tp.gesture != last_g) {
  Serial.printf("[GEST] down=%d gesture=0x%02X x=%u y=%u\n",
                tp.down, tp.gesture, tp.x, tp.y);
  last_g = tp.gesture;
}

  // 4) Fallback debug (WASD) - déjalo apagado para que no interfiera
  // orion_mouse_trackpad_tick(Serial0);

  // 5) Servicio ESP-NOW mínimo
  s3_espnow_loop();

  // Pequeño respiro para no saturar I2C/UART
  delay(1);
}
