#include "weapon.h"
#include "config.h"

static const uint32_t ESC_PERIOD_US = 1000000UL / ESC_PWM_FREQ;   // 20000 us
static const uint32_t ESC_DUTY_MAX  = (1UL << ESC_PWM_BITS) - 1;  // 65535

static float currentPower = 0.0f;  // aktualna moc po rampie (0..1)
static bool  safeToArm    = false; // true dopiero po zobaczeniu wyłączonego ARM

// Zamienia długość impulsu [us] na wypełnienie PWM i wysyła do ESC.
static void writePulseUs(uint16_t us) {
  us = constrain(us, ESC_PULSE_MIN, ESC_PULSE_MAX);
  ledcWrite(ESC_LEDC_CH, (uint32_t)us * ESC_DUTY_MAX / ESC_PERIOD_US);
}

void weaponBegin() {
  ledcSetup(ESC_LEDC_CH, ESC_PWM_FREQ, ESC_PWM_BITS);
  ledcAttachPin(ESC_WEAPON_PIN, ESC_LEDC_CH);
  weaponStop();
  safeToArm = false;
}

void weaponUpdate(float throttle, bool armSwitch) {
  // Bezpieczny start: jeśli przy włączeniu robota przełącznik ARM jest
  // już załączony, broń nie ruszy, dopóki nie przełączysz go na OFF i z powrotem.
  if (!armSwitch) safeToArm = true;

  float target = (armSwitch && safeToArm) ? throttle : 0.0f;

  // Rampa: moc zmienia się stopniowo, bez szarpnięcia przy starcie broni
  if (target > currentPower) {
    currentPower = min(target, currentPower + WEAPON_RAMP_RATE);
  } else {
    currentPower = max(target, currentPower - WEAPON_RAMP_RATE);
  }

  writePulseUs(ESC_PULSE_MIN + (uint16_t)(currentPower * (ESC_PULSE_MAX - ESC_PULSE_MIN)));
}

void weaponStop() {
  currentPower = 0.0f;
  writePulseUs(ESC_PULSE_MIN);
}