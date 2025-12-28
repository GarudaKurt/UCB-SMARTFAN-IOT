#ifndef __OLEDDISPLAY__H
#define __OLEDDISPLAY__H

#include <Arduino.h>

class OLEDDISPLAY {
  public:
    OLEDDISPLAY();
    ~OLEDDISPLAY();

    void initDisplay();
    void showDisplay(int speed, const char * mode = NULL);
    void landingDisplay(const char * message);

  private:
    void drawFan(uint8_t frame);
    uint8_t animFrame = 0;
};

#endif
