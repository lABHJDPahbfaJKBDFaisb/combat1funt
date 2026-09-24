#include "motors.h"
#include "config.h"

// Maksymalna wartość PWM dla ustawionej rozdzielczości (10 bit -> 1023)
static const uint32_t PWM_MAX = (1 << MOTOR_PWM_BITS) - 1;

// Martwa strefa z przeskalowaniem — ruch zaczyna się płynnie od zera
// zaraz za granicą strefy (przeniesione z kodu Arduino IDE).
static float applyDeadzone(float v, float dz) {
  if (fabsf(v) < dz) return 0.0f;
  float sign = (v > 0) ? 1.0f : -1.0f;
  return sign * (fabsf(v) - dz) / (1.0f - dz);
}

// Ustawia jeden kanał mostka: kierunek przez IN1/IN2, prędkość przez PWM.
// speed: -1.0 (pełny wstecz) .. 1.0 (pełny przód)
static void setMotor(uint8_t in1, uint8_t in2, uint8_t ledcCh, float speed) {
  speed = constrain(speed, -1.0f, 1.0f) * MOTOR_VOLTAGE_LIMIT;

  if (speed >= 0) {
    digitalWrite(in1, HIGH);
    digitalWrite(in2, LOW);
  } else {
    digitalWrite(in1, LOW);
    digitalWrite(in2, HIGH);
  }

  // Przy speed = 0 PWM = 0, a TB6612 przy IN1=HIGH, IN2=LOW, PWM=LOW
  // zwiera uzwojenia -> silnik hamuje (tak samo jak w kodzie z Arduino IDE).
  ledcWrite(ledcCh, (uint32_t)(fabsf(speed) * PWM_MAX));
}

void motorsBegin() {
  pinMode(AIN1_PIN, OUTPUT);
  pinMode(AIN2_PIN, OUTPUT);
  pinMode(BIN1_PIN, OUTPUT);
  pinMode(BIN2_PIN, OUTPUT);

  ledcSetup(MOTOR_LEDC_CH_A, MOTOR_PWM_FREQ, MOTOR_PWM_BITS);
  ledcSetup(MOTOR_LEDC_CH_B, MOTOR_PWM_FREQ, MOTOR_PWM_BITS);
  ledcAttachPin(PWMA_PIN, MOTOR_LEDC_CH_A);
  ledcAttachPin(PWMB_PIN, MOTOR_LEDC_CH_B);

  motorsStop();
}

void motorSetLeft(float speed) {
  if (INVERT_LEFT_MOTOR) speed = -speed;
  setMotor(AIN1_PIN, AIN2_PIN, MOTOR_LEDC_CH_A, speed);
}

void motorSetRight(float speed) {
  if (INVERT_RIGHT_MOTOR) speed = -speed;
  setMotor(BIN1_PIN, BIN2_PIN, MOTOR_LEDC_CH_B, speed);
}

void motorsDrive(float throttle, float steering) {
  throttle = applyDeadzone(throttle, DRIVE_DEADBAND);
  steering = applyDeadzone(steering, DRIVE_DEADBAND);

  // Mieszanie arcade: skręt dodaje się do jednej strony i odejmuje od drugiej
  float left  = throttle + steering;
  float right = throttle - steering;

  // Normalizacja — jeśli któraś strona przekracza 1.0, skalujemy obie,
  // żeby zachować proporcję (i promień skrętu)
  float maxMag = max(fabsf(left), fabsf(right));
  if (maxMag > 1.0f) {
    left  /= maxMag;
    right /= maxMag;
  }

  motorSetLeft(left);
  motorSetRight(right);
}

void motorsStop() {
  motorSetLeft(0.0f);
  motorSetRight(0.0f);
}