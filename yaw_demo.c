
#include <math.h>

const byte STEP_PIN = 3;
const byte DIR_PIN  = 4;
const byte EN_PIN   = 5;

const int MOTOR_STEPS = 200;
const int MICROSTEPS = 8;
const int GEAR_RATIO = 10;

const float SWEEP_ANGLE = 60.0f;

const unsigned long HALF_CYCLE = 20000000UL;
const unsigned long FULL_CYCLE = 2UL * HALF_CYCLE;
const unsigned long MIN_STEP_INTERVAL = 1200UL;

const long SWEEP_STEPS = lroundf(
  MOTOR_STEPS * MICROSTEPS * GEAR_RATIO *
  SWEEP_ANGLE / 360.0f
);

long currentStep = 0;
unsigned long startTime = 0;
unsigned long lastStepTime = 0;

bool running = false;
bool stopped = false;

void setup() {
  digitalWrite(EN_PIN, HIGH);

  pinMode(EN_PIN, OUTPUT);
  pinMode(STEP_PIN, OUTPUT);
  pinMode(DIR_PIN, OUTPUT);

  digitalWrite(STEP_PIN, LOW);
  digitalWrite(DIR_PIN, LOW);

  Serial.begin(115200);
  Serial.println("Yaw ready");
  Serial.println("g = start, x = stop");
}

void loop() {
  while (Serial.available()) {
    char command = Serial.read();

    if (command == 'x') {
      running = false;
      stopped = true;
      digitalWrite(EN_PIN, HIGH);
      Serial.println("STOPPED - reset required");
    }

    if (command == 'g' && !running && !stopped) {
      currentStep = 0;
      digitalWrite(EN_PIN, LOW);
      delay(10);

      startTime = micros();
      lastStepTime = startTime;
      running = true;

      Serial.println("Sweep started");
    }
  }

  if (!running) return;

  unsigned long now = micros();
  unsigned long elapsed =
    (now - startTime) % FULL_CYCLE;

  float phase =
    2.0f * PI * (float)elapsed / (float)FULL_CYCLE;

  float fraction =
    0.5f * (1.0f - cosf(phase));

  long targetStep =
    lroundf(SWEEP_STEPS * fraction);

  if (targetStep == currentStep) return;

  if (now - lastStepTime < MIN_STEP_INTERVAL) return;

  bool forward = targetStep > currentStep;

  digitalWrite(DIR_PIN, forward ? HIGH : LOW);
  delayMicroseconds(5);

  digitalWrite(STEP_PIN, HIGH);
  delayMicroseconds(5);
  digitalWrite(STEP_PIN, LOW);

  currentStep += forward ? 1 : -1;
  lastStepTime = micros();
}
