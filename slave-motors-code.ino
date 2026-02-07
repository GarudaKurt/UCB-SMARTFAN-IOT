#include <Servo.h>

Servo servo;

// Pins
const int trigPin  = 5;
const int echPin   = 4;
const int servoPin = 3;

// Servo movement settings
const int SERVO_MIN = 1;
const int SERVO_MAX = 180;
const unsigned long MOVE_DURATION = 2000; // 2 seconds end-to-end

// State variables
int servoAngle = SERVO_MIN;
int direction = 1; // 1 = forward, -1 = reverse
bool startMotion = false;
bool pausedByDistance = false;

// Timing
unsigned long lastMoveTime = 0;
unsigned long stepInterval;

// --------------------------------------------------

void setup() {
  Serial.begin(115200);

  pinMode(trigPin, OUTPUT);
  pinMode(echPin, INPUT);

  servo.attach(servoPin);
  servo.write(90);             
  stepInterval = MOVE_DURATION / (SERVO_MAX - SERVO_MIN);
}

// --------------------------------------------------

float getDistanceCM() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echPin, HIGH, 30000);
  if (duration == 0) return 999;

  return duration * 0.0343 / 2.0;
}

// --------------------------------------------------

void loop() {

  // 🔹 SERIAL COMMAND HANDLING
  if (Serial.available()) {
    char cmd = Serial.read();

    if (cmd == 'S') {
      startMotion = true;      
    }

    if (cmd == 'X') {
      startMotion = false;    
      servoAngle = 90;
      servo.write(90);      
    }
  }

  float distance = getDistanceCM();

  pausedByDistance = (distance < 150);

  if (startMotion && !pausedByDistance) {
    unsigned long now = millis();

    if (now - lastMoveTime >= stepInterval) {
      lastMoveTime = now;

      servoAngle += direction;

      // Reverse at limits
      if (servoAngle >= SERVO_MAX) {
        servoAngle = SERVO_MAX;
        direction = -1;
      } 
      else if (servoAngle <= SERVO_MIN) {
        servoAngle = SERVO_MIN;
        direction = 1;
      }

      servo.write(servoAngle);
    }
  }

  delay(50); // small delay for stability (non-blocking logic preserved)
}
