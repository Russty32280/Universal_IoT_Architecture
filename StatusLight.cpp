//StatusLight.cpp

#include "StatusLight.h"

//constructor defined
StatusLight::StatusLight(int pinA, int pinB, int pinC)
: PIN_A(pinA), PIN_B(pinB), PIN_C(pinC) {}

void StatusLight::setupStatusLight()
{
  pinMode(PIN_A, OUTPUT);
  pinMode(PIN_B, OUTPUT);
  pinMode(PIN_C, OUTPUT);
}

void StatusLight::setSolid(int pin)
{
  //allows for redundant calls without toggle
  if(digitalRead(pin) == LOW)
  {
    digitalWrite(PIN_A, (pin == PIN_A) ? HIGH : LOW);
    digitalWrite(PIN_B, (pin == PIN_B) ? HIGH : LOW);
    digitalWrite(PIN_C, (pin == PIN_C) ? HIGH : LOW);
  }
}

void StatusLight::setBlink(int pin)
{
  digitalWrite(PIN_A, LOW);
  digitalWrite(PIN_B, LOW);
  digitalWrite(PIN_C, LOW);

  unsigned long startTime = millis();
  int flashCount = 0;
  bool isOn = false;

  while (flashCount < NUM_FLASHES * 2) // ensures always an even number
  {
    if (millis() - startTime >= FLASH_DELAY)
    {
      // Toggle the LED state of the specified pin
      digitalWrite(pin, isOn ? LOW : HIGH);
      isOn = !isOn;

      // Reset start time for the next flash
      startTime = millis();
      flashCount++;
    }
  }
digitalWrite(pin, LOW);
}

void StatusLight::turnOff()
{
    digitalWrite(PIN_A, LOW);
    digitalWrite(PIN_B, LOW);
    digitalWrite(PIN_C, LOW);
}