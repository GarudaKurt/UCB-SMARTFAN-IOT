#include "oledDisplay.h"

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

// ===== MATCH YOUR WORKING CODE =====
#define i2c_Address 0x3C
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET   -1

// IMPORTANT: keep name DIFFERENT from your class instance
static Adafruit_SH1106G display(
  SCREEN_WIDTH,
  SCREEN_HEIGHT,
  &Wire,
  OLED_RESET
);

OLEDDISPLAY::OLEDDISPLAY() {}
OLEDDISPLAY::~OLEDDISPLAY() {}

void OLEDDISPLAY::initDisplay() {
  delay(250);  // REQUIRED (from Adafruit example)

  if (!display.begin(i2c_Address, true)) {
    Serial.println(F("SH1106 OLED init failed"));
    while (1);
  }

  display.clearDisplay();
  display.display();
}

void OLEDDISPLAY::showDisplay(int speed, const char * mode) {
  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
  display.setCursor(0, 0);
  display.println("FAN SPEED");

  display.setTextSize(2);
  display.setCursor(0, 16);
  if(speed <= 0)
    speed = 180;
  display.print(speed);
  display.print(" %");

  display.setTextSize(1);
  display.setCursor(0, 40);  // place below the speed
  display.print("Mode: ");
  display.println(mode);


  if (speed > 0) {
    animFrame = (animFrame + 1) % 4;
    drawFan(animFrame);
  }

  display.display();
}

void OLEDDISPLAY::landingDisplay(const char *message) {
  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
  display.setCursor(0, 0);
  display.println(message);
  display.display();   
}

void OLEDDISPLAY::drawFan(uint8_t frame) {
  int cx = 96;
  int cy = 32;

  // hub
  display.fillCircle(cx, cy, 3, SH110X_WHITE);

  switch (frame) {
    case 0:
      display.drawLine(cx, cy, cx + 16, cy, SH110X_WHITE);
      display.drawLine(cx, cy, cx - 16, cy, SH110X_WHITE);
      break;
    case 1:
      display.drawLine(cx, cy, cx + 12, cy - 12, SH110X_WHITE);
      display.drawLine(cx, cy, cx - 12, cy + 12, SH110X_WHITE);
      break;
    case 2:
      display.drawLine(cx, cy, cx, cy + 16, SH110X_WHITE);
      display.drawLine(cx, cy, cx, cy - 16, SH110X_WHITE);
      break;
    case 3:
      display.drawLine(cx, cy, cx - 12, cy - 12, SH110X_WHITE);
      display.drawLine(cx, cy, cx + 12, cy + 12, SH110X_WHITE);
      break;
  }
}
