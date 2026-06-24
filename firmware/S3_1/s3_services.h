#pragma once
#include <Arduino.h>
#include <stdint.h>

void s3_send_heartbeat(HardwareSerial& U);
void s3_send_orion_text(HardwareSerial& U, const char* txt); // ORION_EVT_TEXT (0x01)
void s3_console_poll_and_send_text(HardwareSerial& U);
