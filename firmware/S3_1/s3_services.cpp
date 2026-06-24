#include "s3_services.h"
#include "horio_proto.h"
#include "orion_input_events.h" // para TGT_PC_HID y offsets (lo creamos abajo)

static unsigned long tHB = 0;
static uint8_t orion_seq = 0;

void s3_send_heartbeat(HardwareSerial& U) {
  uint32_t now = millis();
  if (now - tHB <= 1000) return;
  tHB = now;

  uint32_t up   = now;
  uint32_t heap = ESP.getFreeHeap();

  uint8_t payload[8] = {
    (uint8_t)up,   (uint8_t)(up>>8),   (uint8_t)(up>>16),   (uint8_t)(up>>24),
    (uint8_t)heap, (uint8_t)(heap>>8), (uint8_t)(heap>>16), (uint8_t)(heap>>24)
  };

  send_frame(U, ID_SELF, ID_P4, EVT, 0x00, 0, CMD_HEARTBEAT, payload, sizeof(payload));
}

void s3_send_orion_text(HardwareSerial& U, const char* txt) {
  // ORION_EVT_TEXT = 0x01 (ya probado)
  const uint8_t ORION_EVT_TEXT = 0x01;

  uint16_t n = (uint16_t)strlen(txt);
  if (n > 200) n = 200;

  uint8_t data[ORION_OFF_PAYLOAD + 200];
  data[ORION_OFF_EVT_ID] = ORION_EVT_TEXT;
  data[ORION_OFF_TARGET] = TGT_PC_HID;      // tú lo estás usando para pruebas
  data[ORION_OFF_SEQ]    = orion_seq++;
  data[ORION_OFF_FLAGS]  = 0;

  memcpy(&data[ORION_OFF_PAYLOAD], txt, n);

  send_frame(U, ID_SELF, ID_P4, EVT, 0x00, 0, CMD_ORION_EVENT, data, ORION_OFF_PAYLOAD + n);
}

void s3_console_poll_and_send_text(HardwareSerial& U) {
  static char line[128];
  static uint8_t idx = 0;

  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\r') continue;

    if (c == '\n') {
      if (idx > 0) {
        line[idx] = 0;
        s3_send_orion_text(U, line);
        idx = 0;
      }
    } else if (idx < sizeof(line) - 1) {
      line[idx++] = c;
    }
  }
}
