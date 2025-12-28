#ifndef __CONFIGFIREBASE__H
#define __CONFIGFIREBASE__H

#include <Arduino.h>

class CONFIGFIREBASE {
  public:
    CONFIGFIREBASE();
    bool initFirebase();
    int remoteSpeed();   // returns 0–255, or -1 if no update
    bool hasPowerOn();
    bool statusMode();
  private:
    bool getBoolValue(const char* path);
    const char * error = NULL;
    void initTime();
};

#endif
