#include <Arduino.h>
#include "config.h"
#include "crsf.h"
#include "motors.h"

const int melody[] = {523, 659, 784, 1047, 784}; // C5 E5 G5 C6 G5
const int noteDurationMs = 150;
const int notePauseMs = 50;

void playStartupMelody() {
  for (int note : melody) {
    tone(BUZZER_PIN, note, noteDurationMs);
    delay(noteDurationMs + notePauseMs);
  }
  noTone(BUZZER_PIN);
}

void setup() {
  motorsBegin(); //bezpieczny stan silnikow;
  
  Serial.begin(115200);
  delay(1500);

  playStartupMelody();

  crsfBegin();

  Serial.println("pora nakurwiać.");
}

void loop() {
  crsfUpdate();

#if DEBUG_CRSF
  crsfPrintDebug();
#endif

  if (!crsfLinkOk()) {
    motorsStop();   // failsafe: brak łącza -> napęd stoi
    // tu później: stop broni
    delay(5);
    return;
  }

  // --- napęd: prawy drążek ---
  float steering = crsfToFloat(crsfGetChannel(CH_STEERING));
  float throttle = crsfToFloat(crsfGetChannel(CH_THROTTLE));
  motorsDrive(throttle, steering);

  // --- broń: dojdzie w weapon.cpp ---
  // float weapon = crsfToFloatUnipolar(crsfGetChannel(CH_WEAPON));
  // bool  armed  = crsfToFloatUnipolar(crsfGetChannel(CH_ARM)) > 0.5f;

  delay(4);  // ~250 Hz pętli sterującej
}