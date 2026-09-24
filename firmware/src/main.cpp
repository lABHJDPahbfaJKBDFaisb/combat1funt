#include <Arduino.h>
#include "config.h"
#include "crsf.h"
#include "motors.h"
#include "weapon.h"

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
  weaponBegin();
  setToneChannel(TONE_LEDC_CH);
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
    weaponStop();   // i broń też
    delay(5);
    return;
  }

  // --- napęd: prawy drążek ---
  float steering = crsfToFloat(crsfGetChannel(CH_STEERING));
  float throttle = crsfToFloat(crsfGetChannel(CH_THROTTLE));
  motorsDrive(throttle, steering);

  // --- broń: lewy drążek + przełącznik ARM (kanał 6) ---
  float weaponThrottle = crsfToFloatUnipolar(crsfGetChannel(CH_WEAPON));
  bool  armSwitch      = crsfToFloatUnipolar(crsfGetChannel(CH_ARM)) > 0.75f;
  weaponUpdate(weaponThrottle, armSwitch);

  delay(4);  // ~250 Hz pętli sterującej
}