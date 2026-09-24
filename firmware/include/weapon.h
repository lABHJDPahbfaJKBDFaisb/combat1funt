#pragma once

#include <Arduino.h>

// ============================================================================
// Broń — silnik BLDC przez ESC BLHeli_S, sygnał serwo PWM (1000–2000 us)
// ============================================================================

// Uruchamia PWM dla ESC i od razu wysyła impuls minimalny (ESC się uzbraja).
void weaponBegin();

// throttle: 0.0 .. 1.0 (lewy drążek), armSwitch: stan przełącznika ARM.
// Wywoływać w każdej iteracji loop().
void weaponUpdate(float throttle, bool armSwitch);

// Natychmiastowe odcięcie broni (failsafe).
void weaponStop();