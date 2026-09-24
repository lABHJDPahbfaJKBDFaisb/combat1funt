#include "crsf.h"
#include "config.h"

#define CRSF_SYNC_BYTE     0xC8
#define CRSF_TYPE_CHANNELS 0x16
#define CRSF_MAX_FRAME_LEN 64

static HardwareSerial CrsfSerial(1);  // UART1

static uint16_t crsfChannels[CRSF_NUM_CHANNELS];
static unsigned long lastCrsfFrameMs = 0;

static uint32_t crsfByteCount = 0;        // ile bajtów w ogóle przyszło po UART
static uint32_t crsfValidFrameCount = 0;  // ile poprawnych (CRC OK) ramek kanałów

enum CrsfParseState { WAIT_SYNC, WAIT_LEN, WAIT_DATA };
static CrsfParseState crsfState = WAIT_SYNC;
static uint8_t crsfBuf[CRSF_MAX_FRAME_LEN];
static uint8_t crsfFrameLen = 0;
static uint8_t crsfIdx = 0;

// ============================================================================
// CRC8 (DVB-S2, poly 0xD5) — wymagany do weryfikacji ramek CRSF
// ============================================================================
static uint8_t crsfCrc8(uint8_t crc, uint8_t a) {
  crc ^= a;
  for (uint8_t i = 0; i < 8; i++) {
    crc = (crc & 0x80) ? (uint8_t)((crc << 1) ^ 0xD5) : (uint8_t)(crc << 1);
  }
  return crc;
}

static void crsfUnpackChannels(const uint8_t *payload) {
  crsfChannels[0]  = ((payload[0] | payload[1] << 8)) & 0x07FF;
  crsfChannels[1]  = ((payload[1] >> 3 | payload[2] << 5)) & 0x07FF;
  crsfChannels[2]  = ((payload[2] >> 6 | payload[3] << 2 | payload[4] << 10)) & 0x07FF;
  crsfChannels[3]  = ((payload[4] >> 1 | payload[5] << 7)) & 0x07FF;
  crsfChannels[4]  = ((payload[5] >> 4 | payload[6] << 4)) & 0x07FF;
  crsfChannels[5]  = ((payload[6] >> 7 | payload[7] << 1 | payload[8] << 9)) & 0x07FF;
  crsfChannels[6]  = ((payload[8] >> 2 | payload[9] << 6)) & 0x07FF;
  crsfChannels[7]  = ((payload[9] >> 5 | payload[10] << 3)) & 0x07FF;
  crsfChannels[8]  = ((payload[11] | payload[12] << 8)) & 0x07FF;
  crsfChannels[9]  = ((payload[12] >> 3 | payload[13] << 5)) & 0x07FF;
  crsfChannels[10] = ((payload[13] >> 6 | payload[14] << 2 | payload[15] << 10)) & 0x07FF;
  crsfChannels[11] = ((payload[15] >> 1 | payload[16] << 7)) & 0x07FF;
  crsfChannels[12] = ((payload[16] >> 4 | payload[17] << 4)) & 0x07FF;
  crsfChannels[13] = ((payload[17] >> 7 | payload[18] << 1 | payload[19] << 9)) & 0x07FF;
  crsfChannels[14] = ((payload[19] >> 2 | payload[20] << 6)) & 0x07FF;
  crsfChannels[15] = ((payload[20] >> 5 | payload[21] << 3)) & 0x07FF;
}

void crsfBegin() {
  for (uint8_t i = 0; i < CRSF_NUM_CHANNELS; i++) {
    crsfChannels[i] = CRSF_MID;
  }
  CrsfSerial.begin(CRSF_BAUD, SERIAL_8N1, CRSF_RX_PIN, CRSF_TX_PIN);

  // Wymuszamy stan "brak zasięgu" od razu po starcie, dopóki nie przyjdą prawdziwe dane.
  lastCrsfFrameMs = millis() - FAILSAFE_TIMEOUT_MS - 100;
}

void crsfUpdate() {
  while (CrsfSerial.available()) {
    uint8_t b = (uint8_t)CrsfSerial.read();
    crsfByteCount++;

    switch (crsfState) {
      case WAIT_SYNC:
        if (b == CRSF_SYNC_BYTE) {
          crsfBuf[0] = b;
          crsfIdx = 1;
          crsfState = WAIT_LEN;
        }
        break;

      case WAIT_LEN:
        crsfFrameLen = b;  // długość = [typ] + [payload] + [crc]
        if (crsfFrameLen < 2 || crsfFrameLen > CRSF_MAX_FRAME_LEN - 2) {
          crsfState = WAIT_SYNC;  // nieprawidłowa długość, resync
          break;
        }
        crsfBuf[1] = b;
        crsfIdx = 2;
        crsfState = WAIT_DATA;
        break;

      case WAIT_DATA:
        crsfBuf[crsfIdx++] = b;
        if (crsfIdx >= crsfFrameLen + 2) {
          // mamy całą ramkę: crsfBuf[2] = typ, dalej payload, ostatni bajt = CRC
          uint8_t type = crsfBuf[2];
          uint8_t crcReceived = crsfBuf[crsfFrameLen + 1];

          uint8_t crcCalc = 0;
          for (uint8_t i = 2; i < crsfFrameLen + 1; i++) {
            crcCalc = crsfCrc8(crcCalc, crsfBuf[i]);
          }

          if (crcCalc == crcReceived && type == CRSF_TYPE_CHANNELS) {
            crsfUnpackChannels(&crsfBuf[3]);
            lastCrsfFrameMs = millis();
            crsfValidFrameCount++;
          }

          crsfState = WAIT_SYNC;
        }
        break;
    }
  }
}

bool crsfLinkOk() {
  return (millis() - lastCrsfFrameMs) < FAILSAFE_TIMEOUT_MS;
}

uint16_t crsfGetChannel(uint8_t ch) {
  if (ch >= CRSF_NUM_CHANNELS) return CRSF_MID;
  return crsfChannels[ch];
}

float crsfToFloat(uint16_t raw) {
  float v = ((float)raw - CRSF_MID) / (float)(CRSF_MID - CRSF_MIN);
  if (v > 1.0f) v = 1.0f;
  if (v < -1.0f) v = -1.0f;
  return v;
}

float crsfToFloatUnipolar(uint16_t raw) {
  float v = ((float)raw - CRSF_MIN) / (float)(CRSF_MAX - CRSF_MIN);
  if (v > 1.0f) v = 1.0f;
  if (v < 0.0f) v = 0.0f;
  return v;
}

void crsfPrintDebug() {
  static unsigned long lastDebugMs = 0;
  if (millis() - lastDebugMs < 200) return;
  lastDebugMs = millis();

  Serial.print("link=");
  Serial.print(crsfLinkOk() ? "OK" : "BRAK");
  Serial.print("  bajtowUART=");
  Serial.print(crsfByteCount);
  Serial.print("  poprawnychRamek=");
  Serial.print(crsfValidFrameCount);
    Serial.print("  CH1-6 [%]: ");
  for (uint8_t i = 0; i < 6; i++) {
    int pct = (int)lroundf(crsfToFloat(crsfChannels[i]) * 100.0f);
    Serial.printf("%5d", pct);
  }
  Serial.println();
}
