#pragma once
#include <Arduino.h>
#include <stdint.h>

// UART / Protocol constants
static const uint32_t BAUD = 460800;

static const uint8_t SOF1 = 0xAA;
static const uint8_t SOF2 = 0x55;
static const uint8_t VER  = 0x01;

enum MsgType : uint8_t { REQ=0x01, RESP=0x02, EVT=0x03 };

// IDs (S3_1)
static const uint8_t ID_P4   = 0x01;
static const uint8_t ID_SELF = 0x02;

// UART pins on S3
static const int RX0_PIN = 44;
static const int TX0_PIN = 43;

// Commands (HORIO base)
static const uint16_t CMD_PING       = 0x0001;
static const uint16_t CMD_HEARTBEAT  = 0x8001;
static const uint16_t CMD_TEXT_SEND  = 0x1000;
static const uint16_t CMD_TEXT_ACK   = 0x1001;
// ESP-NOW mínimo (FASE 8.0)
static const uint16_t CMD_S3_ESPNOW_INIT      = 0x3060; // payload[0] = canal
static const uint16_t CMD_S3_ESPNOW_SET_PEER  = 0x3061; // payload[0..5]=MAC, payload[6]=encrypt
static const uint16_t CMD_S3_ESPNOW_TEST_SEND = 0x3062; // sin payload

// ORION command (ya congelado en tu base)
static const uint16_t CMD_ORION_EVENT = 0x2070;

// Utils
uint16_t crc16_ccitt_false(const uint8_t* data, size_t len);
void put_le16(uint8_t* p, uint16_t v);
uint16_t get_le16(const uint8_t* p);

// Sender (tu misma función)
void send_frame(HardwareSerial& U, uint8_t src, uint8_t dst, MsgType type, uint8_t flags,
                uint16_t msg_id, uint16_t cmd, const uint8_t* data, uint16_t len);
