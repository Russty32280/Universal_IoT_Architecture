//Gabe Dolce
//10/3/23
//This code takes the basic MQQT Example and has it send sensor data.
#include "Adafruit_SHT4x.h" //to read sensor for sending
#include <WiFi.h>
#include <MQTT.h>

WiFiClient net;
MQTTClient client;

unsigned long lastMillis = 0;

//Wifi config
const char ssid[] = "RowanWiFi";
const char pass[] = ""; //The SSID and password of your Wifi network

//MQTT-Broker config
const char mqttClientId[] = "Gabe"; //The MQTT Client ID. Name you want to be seen as
const char mqttUsername[] = "rusmartlabclinic"; //will always be this for our instance
const char mqttPassword[] = "ESP32Connect"; //password for your MQTT-Broker


void connect() {
  Serial.print("checking wifi...");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }

  Serial.print("\nconnecting to client...");
  while (!client.connect(mqttClientId, mqttUsername, mqttPassword)) {
    Serial.print(".");
    delay(1000);
  }

  Serial.println("\nconnected!");

  //put any topics you want to subscribe to here
  //client.subscribe("Chatting"); //subscribe to a topic
}

void messageReceived(String &topic, String &payload) {
  //react on MQTT commands with according functions
  Serial.println("incoming: " + topic + " - " + payload);
}


Adafruit_SHT4x sht4 = Adafruit_SHT4x();
float tempC = 0;
float humid = 0;
void setup()
{
  Serial.begin(115200);
  WiFi.begin(ssid);
  client.begin("rusmartlabclinic.cloud.shiftr.io", net); //put the IP-Address of your broker here
  client.onMessage(messageReceived);

  connect();

  while (!Serial)
    delay(10);     // will pause Zero, Leonardo, etc until serial console opens

  Serial.println("Adafruit SHT4x test");
  if (! sht4.begin()) {
    Serial.println("Couldn't find SHT4x");
    while (1) delay(1);
  }
  Serial.println("Found SHT4x sensor");
  Serial.print("Serial number 0x");
  Serial.println(sht4.readSerial(), HEX);

  // You can have 3 different precisions, higher precision takes longer
  sht4.setPrecision(SHT4X_LOW_PRECISION);
 
 //Heats the sensor to get better readings
  sht4.setHeater(SHT4X_NO_HEATER);
  
}


void loop() {
  client.loop();
  if (!client.connected()) {
    connect();
  }
  sensors_event_t humidity, temp; //how the sensor needs to work
  
  uint32_t timestamp = millis();
  sht4.getEvent(&humidity, &temp);// populate temp and humidity objects with fresh data
  timestamp = millis() - timestamp;

  tempC = temp.temperature; //grabs the reading as a float to transfer.
  Serial.print("Temperature(C):"); //This allows for the serial plotter to be used arduino
  Serial.print(tempC); //change to println when only one
  Serial.print(",");

  humid = humidity.relative_humidity;
  Serial.print("Humidity(%):");
  Serial.println(humid); //change to println when only one. helps with serial plotter.

  client.publish("/Sensors/Temperature", String(tempC)); //Must send either a char array or String. Cast the int.
  client.publish("/Sensors/Humidity", String(humid)); //While not necessary, I wanted to experiment with this hierarchal structure.
  client.subscribe("/Sensors/Temperature"); //You can subscribe to the general topic, but you don't get both data. Must specify.
  delay(10000); //10 second delay
  
}