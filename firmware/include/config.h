#pragma once

// ============================================================================
// KONFIGURACJA PINÓW I USTAWIEŃ — wspólna dla wszystkich plików .cpp
// ============================================================================

// --- TB6612FNG: silniki napędowe ---
#define PWMA_PIN   14   // PWM silnika A (lewy)
#define AIN1_PIN   12
#define AIN2_PIN   13
#define PWMB_PIN   9    // PWM silnika B (prawy)
#define BIN1_PIN   11
#define BIN2_PIN   10
// STBY podpięty na sztywno do +3V3 na płytce.

// --- ESC broni (BLDC), sygnał serwo PWM ---
#define ESC_WEAPON_PIN 21
#define ESC_PWM_FREQ 50 //standardowy sygnal servo 50hz;
#define ESC_PWM_BITS      14    // rozdzielczość: ~0,3 us na krok
#define ESC_LEDC_CH       2      // kanał 2 -> inny timer niż napęd (kanały 0/1)
#define ESC_PULSE_MIN     1000   // [us] gaz zero / uzbrajanie
#define ESC_PULSE_MAX     2000   // [us] pełny gaz
#define WEAPON_RAMP_RATE  0.02f  // max zmiana mocy na iterację pętli (~0,2 s od 0 do 100%)

// --- Odbiornik ELRS (CRSF) po UART ---
#define CRSF_RX_PIN 18   // ESP32 RX <- TX odbiornika
#define CRSF_TX_PIN 17   // ESP32 TX -> RX odbiornika (telemetria, opcjonalnie)
#define CRSF_BAUD   420000

// --- Buzzer startowy ---
#define BUZZER_PIN 42
#define TONE_LEDC_CH 6   // kanał dla tone() — osobny timer, żeby nie psuł PWM napędu

// --- Mapowanie kanałów CRSF (indeksy 0-15, AETR, Mode 2) ---
#define CH_STEERING   0   // prawy drążek poziomo: skręt
#define CH_THROTTLE   1   // prawy drążek pionowo: jazda przód/tył
#define CH_WEAPON     2   // lewy drążek pionowo: prędkość broni
#define CH_ARM        5   // kanał 6: przełącznik uzbrojenia broni

// Failsafe — brak poprawnej ramki CRSF przez ten czas = stop
#define FAILSAFE_TIMEOUT_MS   300

// Diagnostyka CRSF na Serial (0 = wyłączona)
#define DEBUG_CRSF 1
 
// --- Napęd: ustawienia PWM i sterowania ---
#define MOTOR_PWM_FREQ     20000  // 20 kHz — poza zakresem słyszalności, TB6612 obsługuje do 100 kHz
#define MOTOR_PWM_BITS     10     // rozdzielczość PWM: 0..1023
#define MOTOR_LEDC_CH_A    0      // kanały LEDC dla silników (0 i 1 dzielą jeden timer)
#define MOTOR_LEDC_CH_B    1

#define DRIVE_DEADBAND      0.04f  // martwa strefa drążków (4%)
#define MOTOR_VOLTAGE_LIMIT 0.81f  // N20 są na 9V, zasilanie 3S = 11,1V -> max 9.0/11.1 PWM
#define INVERT_LEFT_MOTOR  false  // zmień na true, jeśli silnik kręci w złą stronę
#define INVERT_RIGHT_MOTOR false