#pragma once

#include <Arduino.h>

// ============================================================================
// Napęd — dwa silniki DC (N20) przez mostek H TB6612FNG
// ============================================================================

// Konfiguruje piny kierunku i PWM, zostawia silniki zatrzymane.
void motorsBegin();

// Sterowanie "arcade": throttle i steering w zakresie -1.0 .. 1.0
void motorsDrive(float throttle, float steering);

// Ustawia pojedynczy silnik: speed -1.0 (pełny wstecz) .. 1.0 (pełny przód)
void motorSetLeft(float speed);
void motorSetRight(float speed);

// Zatrzymuje oba silniki.
void motorsStop();