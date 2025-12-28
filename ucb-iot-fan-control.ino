#include <Arduino.h>
#include <Wire.h>
#include <ESP32Servo.h>

#include "oledDisplay.h"
#include "configFirebase.h"

Servo servo;

OLEDDISPLAY display;
CONFIGFIREBASE firebase;

const int PIRpin   = 17;
const int servoPin = 18;

// L298N Motor Driver
const int ENA = 25;   // PWM
const int IN1 = 26;
const int IN2 = 27;

const int pwmFreq       = 1000;   // 1 kHz
const int pwmResolution = 8;      // 0–255

unsigned long oledMillis     = 0;
unsigned long firebaseMillis = 0;
unsigned long servoMillis    = 0;
unsigned long prevTime = 0;

const unsigned long OLED_INTERVAL     = 100;
const unsigned long FIREBASE_INTERVAL = 2000;
const unsigned long SERVO_INTERVAL    = 20;

bool firebaseActive = false;
bool isActivateServo = false;

int currentSpeed = 0;
int targetSpeed  = 0;

int servoAngle = 0;
int servoDir   = 1;


void setup() {
  Serial.begin(115200);
  Wire.begin();

  pinMode(PIRpin, INPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);

  // Servo
  servo.attach(servoPin);
  servo.write(90);

  ledcAttach(ENA, pwmFreq, pwmResolution);
  ledcWrite(ENA, 0);

  Serial.println("ESP32 FAN CONTROL START");

  display.initDisplay();
  display.landingDisplay("Initializing...");

  if(!firebase.initFirebase()) {
    pirMode();
  }

}

void loop() {
  unsigned long now = millis();

  bool powerOn = firebase.hasPowerOn();
  bool autoMode = firebase.statusMode();

  if (now - firebaseMillis >= FIREBASE_INTERVAL) {
    firebaseMillis = now;

    int remote = firebase.remoteSpeed();
    if (remote >= 1) {
      targetSpeed = constrain(remote, 0, 255);
      firebaseActive = targetSpeed > 0;
      Serial.print("Firebase speed: ");
      Serial.println(targetSpeed);

      if (firebaseActive) {
        isActivateServo = true;
        prevTime = now;
      }
    }
  }

  if (isActivateServo) {
    servoSweep();
    if (now - prevTime >= 60000) {
      Serial.println("60s passed, checking servo activation again...");
      prevTime = now; 
    }
  }

 if (firebaseActive) {
    Serial.println("Active fan!");
    moveForward(targetSpeed);
  } else {
    if (!powerOn && !autoMode) {
      pirMode();
    }
  }

  if (now - oledMillis >= OLED_INTERVAL) {
    oledMillis = now;
    display.showDisplay(targetSpeed, firebaseActive ? "Automatic" : "Manual");
  }
}


void moveForward(int speed) {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  ledcWrite(ENA, speed);
}
void stopMotor() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  ledcWrite(ENA, 0);
}

void pirMode() {
  int motion = digitalRead(PIRpin);

  if (motion == HIGH) {
    Serial.println("Motion detected!");
    moveForward(180);
    display.showDisplay(180, "Manual");

    isActivateServo = false;
    resetServo(); 
  } else {
    Serial.println("Motion not detected!");
    stopMotor();
  }
}
void servoSweep() {
  unsigned long now = millis();

  if (now - servoMillis >= SERVO_INTERVAL) {
    servoMillis = now;

    servoAngle += servoDir * 10;
    
    if (servoAngle >= 180) {
      servoAngle = 180;
      servoDir = -1; // reverse
    } 
    else if (servoAngle <= 0) {
      servoAngle = 0;
      servoDir = 1;  // forward
    }

    servo.write(servoAngle);
    Serial.print("Servo angle: "); Serial.println(servoAngle);
  }
}


void resetServo() {
  servoAngle = 0;
  servoDir   = 1;
  servo.write(0);
}
