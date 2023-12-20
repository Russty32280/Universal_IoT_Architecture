//This code connects an ESP32 wifi module
//to the specified wifi and to a private shiftr.io instance.
//It will then search for an NCAP to establish communications with.
//Once an NCAP is found. It sends its name to it and creates an uplink
//and downlink channel for communication.

//Boards used :DOIT ESP32 DEVKIT V1 and Node32s

#include <WiFi.h>
#include <NTPClient.h>
#include <MQTT.h>
#include <iostream> 
#include <vector>
using namespace std;

#include "Adafruit_SHT4x.h"
Adafruit_SHT4x sht4 = Adafruit_SHT4x();

WiFiClient net;
MQTTClient client;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "time.nist.gov");

unsigned long lastMillis = 0;

//Wifi config
const char ssid[] = "RowanWiFi";
const char pass[] = ""; //The SSID and password of your Wifi network


//MQTT-Broker config
char mqttClientId[12]; //The MQTT Client ID. Name you want to be seen as
const char mqttUsername[] = "rusmartlabclinic"; //will always be this for our instance
const char mqttPassword[] = "RUSmartLabClinic"; //password for your MQTT-Broker
String client_id; //client id in string form rather than c-string

struct XDCR{
  String name;
  String ID;
  String data;
  XDCR(String str, String id){
    name = str;
    ID = id;
  }
};
const int numChan = 1;
XDCR XDCRList[numChan] = {XDCR("Temperature","0")};

struct NCAP{
  String name;
  bool flag;
  bool isInitialized;
  String uplink(){
    return client_id + "_" + name;
  }
  String downlink(){
    return name + "_" + client_id;
  }
  String response(){
    return name + "_" + "Server_TIM_Discover_Response";
  }
  NCAP(){
    flag = false;
    isInitialized = false;
  }
};
NCAP ncap;

vector<String> subscriptions = {"NCAP_Server_Discover"};
void update_Subscriptions(){
  subscriptions = {"NCAP_Server_Discover"};
  if(ncap.isInitialized){
    subscriptions.push_back(ncap.downlink());
  }
  if(client.connected()){
    for (String topic : subscriptions){
      client.subscribe(topic);
    }
  }
}

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
  //subscribes to all topics
  update_Subscriptions();
}

void messageReceived(String &topic, String &payload) {
  //react on MQTT commands with according functions
  
  if(topic.compareTo(subscriptions.at(0))==0 && !ncap.isInitialized){
    ncap.name = payload;
    ncap.isInitialized = true;
    Serial.println(ncap.name);
    return;
  }
  Serial.println("incoming: " + topic + " - " + payload);
  
  if(ncap.isInitialized && topic.compareTo(ncap.downlink()) == 0)
    {
    parseMessage(payload);
    }
}

void parseMessage(String payload) {
  char formatted [15];
  int count = 0;
  for(int i = 0 ; i < payload.length() ; i++)
  {
    if(payload.charAt(i) != ',')
    {
      formatted[count] = payload.charAt(i);
      count++;
      //Serial.print(formatted[count]);
    }
  }
  String reply;
  if(formatted[0] == '1')
  {
    if(formatted[1] == '6')
    {
      if(formatted[2] == '1')
      {
        String names, IDs;
        for(int i = 0; i < numChan; i++){
          String spacer = "";
          if(i != numChan-1){
            spacer = ";";
          }
          names += XDCRList[i].name + spacer;
          IDs += XDCRList[i].ID + spacer;
        }
        reply = "1,6,2,55,0," + String(formatted[5]) + "," + String(numChan)+ "," + IDs + "," + names;
        Serial.println(IDs);
        Serial.println(names);
      }
    }
  }
  else if(formatted[0] == '2')
  {
    if(formatted[1] == '1')
    {
      if(formatted[2] == '1')
      {
        sensors_event_t humidity, temp;
        sht4.getEvent(&humidity, &temp);// populate temp and humidity objects with fresh data

        timeClient.update();
        String timestamp = timeClient.getFormattedTime();
        Serial.println(timestamp);
        
        XDCRList[int((formatted[6])- '0')].data = String(temp.temperature);
        Serial.println(XDCRList[int((formatted[6])- '0')].data);
        reply = "2,1,2,55,0," + String(formatted[4]) + "," + String(formatted[5]) + "," + String(formatted[6]) + "," + XDCRList[int((formatted[6])- '0')].data + "," + timestamp;
        Serial.println(reply);
      }
    }
  }

  if(reply != NULL)
  {
    client.publish(ncap.uplink(), reply);
    blinky();
  }
}

void blinky(){
  digitalWrite(LED_BUILTIN, HIGH);
  delay(50);
  digitalWrite(LED_BUILTIN, LOW);
  delay(50);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(50);
  digitalWrite(LED_BUILTIN, LOW);
}

void generateName(){
  long randomIdentifier = random(10000000, 99999999);
  String str = String(randomIdentifier);
  char cstr[str.length()+1];
  String(randomIdentifier).toCharArray(cstr, str.length()+1);
  strcat(mqttClientId, "TIM");
  strcat(mqttClientId, cstr);
  client_id = mqttClientId;
}
  
void setup() {
  Serial.begin(115200);

  randomSeed((unsigned) time(NULL));
  generateName();

  pinMode(LED_BUILTIN, OUTPUT);
  blinky();
  
  Serial.println("Adafruit SHT4x test");
  if (! sht4.begin()) {
    Serial.println("Couldn't find SHT4x");
    while (1) delay(1);
  }
  Serial.println("Found SHT4x sensor");
  Serial.print("Serial number 0x");
  Serial.println(sht4.readSerial(), HEX);

  WiFi.begin(ssid);
  //WiFi.begin(ssid, pass);

  timeClient.begin();
  timeClient.setTimeOffset(-18000);
  
  client.begin("rusmartlabclinic.cloud.shiftr.io", net); //put the IP-Address of your broker here
  client.onMessage(messageReceived);

  connect();
}

unsigned long previousMillis = 0;
const int interval = 5000;

void loop() {
  client.loop();
  if (!client.connected()) {
    connect();
  }
  
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval && ncap.isInitialized) {
    previousMillis = currentMillis;
    update_Subscriptions();
    client.publish(ncap.response(), client_id);
  }
  
}
