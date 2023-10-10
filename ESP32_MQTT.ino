//This code connects an ESP32 wifi module
//to the specified wifi and to a private shiftr.io instance 
//It then wait for the user to provide a serial input and publishes 
//it to the chatting topic. It will also print all other messages
//published to that topic to the serial output.

#include <WiFi.h>
#include <MQTT.h>

WiFiClient net;
MQTTClient client;

unsigned long lastMillis = 0;

//Wifi config
const char ssid[] = "RowanWiFi";
const char pass[] = ""; //The SSID and password of your Wifi network

//MQTT-Broker config
const char mqttClientId[] = "TroyESP32"; //The MQTT Client ID. Name you want to be seen as
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
  client.subscribe("Chatting"); //subscribe to a topic
}

void messageReceived(String &topic, String &payload) {
  //react on MQTT commands with according functions
  Serial.println("incoming: " + topic + " - " + payload);
}
  
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  //if your network has a password,
  //comment out first wifi.begin and
  //uncomment the second one.

  WiFi.begin(ssid);
  //WiFi.begin(ssid, pass);
  
  client.begin("rusmartlabclinic.cloud.shiftr.io", net); //put the IP-Address of your broker here
  client.onMessage(messageReceived);

  connect();
}

void loop() {
  client.loop();
  if (!client.connected()) {
    connect();
  }

  const int MAX_MESSAGE_LENGTH = 100; //maximum length of serial input message
  int message_pos = 0;                //indexing through serial input
  char message[MAX_MESSAGE_LENGTH];   //stores serial input message
  
  while(Serial.available() > 0){
    //is something waiting in the serial bus?

    char input = Serial.read(); //reads serial bus and stores it
    
    if(input != '\n' && (message_pos < MAX_MESSAGE_LENGTH - 1)){
      //stores input byte into input message array
      //if it is not a new line char or 100th char
      
      message[message_pos] = input;
      message_pos++;
      
    }else{
      //end of message. Publish the message to the topic.
      
      message[message_pos] = '\0';
      message_pos = 0;
      client.publish("/Chatting", message);
      Serial.println(message);
      
    }
  }
}