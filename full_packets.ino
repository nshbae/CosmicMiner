// remote_control_packet.ino
// 2 Joysticks (4 axes), 2 linear pots, 5 buttons -> 16-byte packet
// Packet: [0xAA][A0L][A0H]...[A5L][A5H][BTN][CHK][0x55]

#include <Arduino.h>

// ====== Serial ======
static const unsigned long BAUD = 115200;

// ====== Pins ======
// Analog
static const uint8_t PIN_J1_X = A0;
static const uint8_t PIN_J1_Y = A1;
static const uint8_t PIN_J2_X = A2;
static const uint8_t PIN_J2_Y = A3;
static const uint8_t PIN_POT1 = A4;
static const uint8_t PIN_POT2 = A5;

// Buttons (INPUT_PULLUP)
static const uint8_t PIN_BTN1 = 2;
static const uint8_t PIN_BTN2 = 3;
static const uint8_t PIN_BTN3 = 4;
static const uint8_t PIN_BTN4 = 5;
static const uint8_t PIN_BTN5 = 6;

// ====== Packet constants ======
static const uint8_t PKT_START = 0xAA;
static const uint8_t PKT_END   = 0x55;
static const uint8_t PKT_SIZE  = 16;

// ====== Timing ======
static const uint16_t SEND_INTERVAL_MS = 5; // ~200Hz
unsigned long lastSendMs = 0;

// ====== Helpers ======
inline void writeUint16LE(uint16_t v) {
  Serial.write((uint8_t)(v & 0xFF));
  Serial.write((uint8_t)((v >> 8) & 0xFF));
}

uint8_t readButtonsBitfield() {
  uint8_t bits = 0;
  bits |= (digitalRead(PIN_BTN1) == LOW) << 0;
  bits |= (digitalRead(PIN_BTN2) == LOW) << 1;
  bits |= (digitalRead(PIN_BTN3) == LOW) << 2;
  bits |= (digitalRead(PIN_BTN4) == LOW) << 3;
  bits |= (digitalRead(PIN_BTN5) == LOW) << 4;
  return bits;
}

void setup() {
  Serial.begin(BAUD);

  pinMode(PIN_BTN1, INPUT_PULLUP);
  pinMode(PIN_BTN2, INPUT_PULLUP);
  pinMode(PIN_BTN3, INPUT_PULLUP);
  pinMode(PIN_BTN4, INPUT_PULLUP);
  pinMode(PIN_BTN5, INPUT_PULLUP);
}

void loop() {
  unsigned long now = millis();
  if (now - lastSendMs < SEND_INTERVAL_MS) return;
  lastSendMs = now;

  // --- Read analogs (0~1023) ---
  uint16_t a0 = analogRead(PIN_J1_X);
  uint16_t a1 = analogRead(PIN_J1_Y);
  uint16_t a2 = analogRead(PIN_J2_X);
  uint16_t a3 = analogRead(PIN_J2_Y);
  uint16_t a4 = analogRead(PIN_POT1);
  uint16_t a5 = analogRead(PIN_POT2);

  // --- Buttons ---
  uint8_t btnBits = readButtonsBitfield();

  // --- Checksum (sum of analog bytes + btnBits) ---
  uint8_t chk = 0;
  uint16_t analogs[6] = {a0,a1,a2,a3,a4,a5};
  for (int i = 0; i < 6; i++) {
    chk += (uint8_t)(analogs[i] & 0xFF);
    chk += (uint8_t)(analogs[i] >> 8);
  }
  chk += btnBits;

  // --- Send packet ---
  Serial.write(PKT_START);

  writeUint16LE(a0);
  writeUint16LE(a1);
  writeUint16LE(a2);
  writeUint16LE(a3);
  writeUint16LE(a4);
  writeUint16LE(a5);

  Serial.write(btnBits);
  Serial.write(chk);
  Serial.write(PKT_END);
}
