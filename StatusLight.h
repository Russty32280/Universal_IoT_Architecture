//StatusLight.h

#ifndef STATUS_LIGHT_H
#define STATUS_LIGHT_H

#include <Arduino.h>

class StatusLight
{
  public:
    //basic constructor takes 3 LED pins
    StatusLight(int pinA, int pinB, int pinC);

    //configures IO pins
    void setupStatusLight();

    //pin number entered is what is set solid
    void setSolid(int pin);

    //pin number is one which flashes
    void setBlink(int pin);

    void turnOff();

  private:
    //store values of each pin number. required
    const int PIN_A;
    const int PIN_B;
    const int PIN_C;

    const int FLASH_DELAY = 500; //delay it is on/off in millis
    const int NUM_FLASHES = 2; //amount of flashes
};

#endif