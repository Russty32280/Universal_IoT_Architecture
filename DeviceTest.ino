#include <LiquidCrystal.h>
#include <WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include "Adafruit_SHT4x.h"
#include "StatusLight.h"

// Define the GPIO pins on the ESP32 connected to the LCD
const int rs = 4, en = 16, d4 = 17, d5 = 5, d6 = 18, d7 = 19;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

// Define your WiFi credentials
const char* ssid = "RowanWiFi";
const char* password = "";

// Define NTP settings
const char* ntpServer = "pool.ntp.org";
const int timeZone = -4; // Eastern Standard Time (EST)

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, ntpServer, timeZone * 3600, 60000);

// Define pins for StatusLight
const int PIN_GREEN = 25;    // Red
const int PIN_RED = 33;  // Green
const int PIN_BLUE = 32;   // Blue
StatusLight myStatusLight(PIN_RED, PIN_GREEN, PIN_BLUE);

Adafruit_SHT4x sht4 = Adafruit_SHT4x();

void setup() {
  Serial.begin(115200);

  // Initialize LCD
  lcd.begin(16, 2); // Adjust according to your LCD's dimensions

  // Setup StatusLight
  myStatusLight.setupStatusLight();

  lcd.print("Initializing...");

  myStatusLight.setSolid(PIN_RED); // Red before connected to wifi
  // Connect to WiFi
  connectToWiFi();

  // Initialize NTP client
  timeClient.begin();
  timeClient.update();

  

  // Setup SHT4x sensor
  if (!sht4.begin()) {
    Serial.println("Couldn't find SHT4x");
    while (1);
    delay(500);
  }
  Serial.println("Found SHT4x sensor");
}

// Flag to track whether NTP time has been successfully updated
bool ntpUpdated = false;

void loop() {
  // Update time from NTP server
  if (!ntpUpdated) {
    ntpUpdated = timeClient.update();
  }

  // Get current time
  unsigned long epochTime = timeClient.getEpochTime();
  struct tm * currentTime = localtime((const time_t*)&epochTime);

  // Format time
  char timeStr[9];
  sprintf(timeStr, "%02d:%02d", currentTime->tm_hour, currentTime->tm_min);

  // Display time on LCD
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Time: ");
  lcd.print(timeStr);

  // Get temperature and humidity from SHT4x sensor
  sensors_event_t humidity, temp;
  sht4.getEvent(&humidity, &temp);

  // Display temperature and humidity on LCD
  lcd.setCursor(0, 1);
  lcd.print("Temp: ");
  lcd.print(temp.temperature);
  lcd.print(" C");

  // Update StatusLight based on WiFi and NTP connection
  if (WiFi.status() != WL_CONNECTED)
  {
    myStatusLight.setSolid(PIN_RED); // Red = no wifi
  } 
  else if(!ntpUpdated)
  {
    myStatusLight.setSolid(PIN_GREEN); // Green = wifi but no server.
    //server updates every minute. If just connected it has to wait a minute to turn blue
  }
  else
  {
    myStatusLight.setSolid(PIN_BLUE); // Blue = updates from server
  }

  delay(10000); // Update every 10 seconds
}

void connectToWiFi() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
}
