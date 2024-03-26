// YourMainSketch.ino

//Example for using the TrafficLight header file
#include <Arduino.h>
#include "StatusLight.h"

const int PIN_A = 25;    // Replace with your actual pin numbers
const int PIN_B = 33; // Replace with your actual pin numbers
const int PIN_C = 32;  // Replace with your actual pin numbers

StatusLight myStatusLight(PIN_A, PIN_B, PIN_C);

//flashing red indicates errors with device
//solid red indicates no connections.

//flashing yellow indicates connecting to wifi
//solid yellow indicates wifi connection secured

//flashing green indicates connecting to MQTT
//solid green indicates MQTT connection secured

void setup()
{
  myStatusLight.setupStatusLight();
}

void loop() {
  myStatusLight.setSolid(PIN_A);
  delay(1000);
  myStatusLight.setSolid(PIN_B);
  delay(1000);
  myStatusLight.setSolid(PIN_C);
  delay(1000);

  myStatusLight.setBlink(PIN_A);
  myStatusLight.setBlink(PIN_B);
  myStatusLight.setBlink(PIN_C);
}

