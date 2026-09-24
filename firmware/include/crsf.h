#pragma once

#include <Arduino.h>

// ============================================================================
// Odbiornik ELRS — protokół CRSF po UART
// ============================================================================

// Zakres surowych wartości CRSF
#define CRSF_MIN 172
#define CRSF_MID 992
#define CRSF_MAX 1811

#define CRSF_NUM_CHANNELS 16

// Uruchamia UART odbiornika i ustawia stan "brak łącza" do pierwszej ramki.
void crsfBegin();

// Wywoływać w każdej iteracji loop() — czyta bajty z UART i parsuje ramki.
void crsfUpdate();

// true, jeśli ostatnia poprawna ramka kanałów przyszła < FAILSAFE_TIMEOUT_MS temu.
bool crsfLinkOk();

// Surowa wartość kanału (172..1811), ch = 0..15
uint16_t crsfGetChannel(uint8_t ch);

// Kanał jako -1.0 .. 1.0 (drążki)
float crsfToFloat(uint16_t raw);

// Kanał jako 0.0 .. 1.0 (gaz broni, przełączniki)
float crsfToFloatUnipolar(uint16_t raw);

// Wypisuje stan łącza i kanały 1-6 na Serial (co najwyżej co 200 ms).
void crsfPrintDebug();
