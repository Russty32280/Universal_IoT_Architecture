//This code connects an ESP32 wifi module
//to the specified wifi and to a private shiftr.io instance.
//It will then search for an NCAP to establish communications with.
//Once an NCAP is found. It sends its name to it and creates an uplink
//and downlink channel.

#include <WiFi.h>
#include <MQTT.h>
#include <iostream> 
#include <vector>
using namespace std;

WiFiClient net;
MQTTClient client;

unsigned long lastMillis = 0;

//Wifi config
const char ssid[] = "RowanWiFi";
const char pass[] = ""; //The SSID and password of your Wifi network


//MQTT-Broker config
char mqttClientId[12]; //The MQTT Client ID. Name you want to be seen as
const char mqttUsername[] = "rusmartlabclinic"; //will always be this for our instance
const char mqttPassword[] = "RUSmartLabClinic"; //password for your MQTT-Broker
String client_id; //client id in string form rather than c-string

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


vector<String> subscriptions = {"NCAP_Server_TIM_Discover"};
void update_Subscriptions(){
  subscriptions = {"NCAP_Server_TIM_Discover"};
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
  Serial.println("incoming: " + topic + " - " + payload);
  
  if(topic.compareTo(subscriptions.at(0))==0 && !ncap.isInitialized){
    ncap.name = payload;
    ncap.isInitialized = true;
    Serial.println(ncap.name);
    return;
  }
  if(topic.compareTo(ncap.downlink())==0){
    ncap.flag = true;
    return;
  }
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

  WiFi.begin(ssid);
  //WiFi.begin(ssid, pass);
  
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
  
  if(ncap.flag){
    client.publish(ncap.uplink(), "Hi");
    ncap.flag = false;
  }
}
