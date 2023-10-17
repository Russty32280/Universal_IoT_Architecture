//This code connects an ESP32 wifi module
//to a private shiftr.io instance and publishes a
//message to a topic in that instance.

#include <ESP8266WiFi.h>
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
    delay(10);

    if(input != '\n' && (message_pos < MAX_MESSAGE_LENGTH - 1)){
      //stores input byte into input message array
      //if it is not a new line char or 100th char
      
      message[message_pos] = input;
      message_pos++;
      
    }else{
      //end of message. will publish the message and clear it
      
      message[message_pos] = '\0';
      message_pos = 0;
      client.publish("/Chatting", message);
      Serial.println(message);

      /*for(int i=0; i<MAX_MESSAGE_LENGTH-1; i++){
        message[i] = '\0';
      }*/
      
    }
  }
}