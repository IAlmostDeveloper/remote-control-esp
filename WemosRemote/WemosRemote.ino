#include <SPI.h>
#include <Wire.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <Arduino.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <IRrecv.h>
#include <IRutils.h>

const uint16_t kIrLed = 12;
const uint16_t kRecvPin = 14;

IRsend irsend(kIrLed);
IRrecv irrecv(kRecvPin);
decode_results results;


const char *ssid = "xxxxxxxxxxxxx";
const char *pass = "xxxxxxxxxxxxx";

const char *mqtt_server = "xxxxxxxxxxxxx";
const int mqtt_port = 1883;
const char *mqtt_user = "xxxxxxxxxxxxx";
const char *mqtt_pass = "xxxxxxxxxxxxx";
const char* mqtt_topics[] = { "remoteControl/devices/1/code/RC5Controller", 
                              "remoteControl/devices/1/code/NECController", 
                              "remoteControl/devices/1/receive"};

WiFiClient wclient; 
void callback(char* topic, byte* payload, unsigned int length);

void printMessage(byte* payload, unsigned int length){
  for (int i = 0; i < length; i++)
      Serial.print((char)payload[i]);
  Serial.print("\n");
}

unsigned long getCodeFromPayload(byte* payload, unsigned int length){
  String value = "";
  for(int i=0;i<length;i++)
    value += (char)payload[i];
  char arr[value.length() + 1];
  value.toCharArray(arr, value.length() + 1);
  unsigned long code = strtol(arr, NULL, 16);
  return code;
}

void sendCode(byte* payload, unsigned int length, String encoding){
  unsigned long code = getCodeFromPayload(payload, length);
  Serial.println(code, HEX);
  if(encoding=="RC5")
    irsend.sendRC5(code);
  if(encoding=="NEC")
    irsend.sendNEC(code);
  for(int i=0;i<4;i++){
    digitalWrite(LED_BUILTIN, 1-digitalRead(LED_BUILTIN));
    delay(50);
  }
}

unsigned long receiveCode(){
  irrecv.enableIRIn();
  while(1>0){
    if (irrecv.decode(&results)) {
    serialPrintUint64(results.value, HEX);
    Serial.println("");
    irrecv.resume();
    return results.value;
    }
    delay(10);
  }
}

PubSubClient client(mqtt_server, mqtt_port, callback, wclient);

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.println("]");
  String encoding;
  String currentTopic = String(topic);
  if(currentTopic.endsWith("Controller")){
    if(currentTopic.endsWith("/RC5Controller"))
      encoding = "RC5";
    if(currentTopic.endsWith("/NECController"))
      encoding = "NEC";

    sendCode(payload, length, encoding);
  }  
  if(currentTopic.endsWith("/receive")){
    unsigned long receivedCode = receiveCode();
    serialPrintUint64(receivedCode, HEX);
    StaticJsonBuffer<200> jsonBuffer;
    JsonObject& obj = jsonBuffer.createObject();
    String data = "";
    byte results[128];
    obj["code"] = receivedCode;
    obj.printTo(data);
    for(int i=0;i<data.length();i++){
      results[i] = (byte)data[i];
    }
    String sendTopic = "";
    for(int i=0;i<length;i++)
      sendTopic += (char) payload[i];
    char sendTopicArray[sendTopic.length() + 1];
    sendTopic.toCharArray(sendTopicArray, sendTopic.length() + 1);
    Serial.print("Topic: ");
    Serial.println(sendTopicArray);
    client.publish(sendTopicArray, results, data.length());
    Serial.println(data);
    Serial.println(data.length());
  }
}

void subscribeToAllTopics(){
  for(int i=0;i<(sizeof(mqtt_topics)/sizeof(*mqtt_topics));i++)
    client.subscribe(mqtt_topics[i]);
}

void connectToWiFi(){
  if (WiFi.status() != WL_CONNECTED) {
    Serial.print("Connecting to ");
    Serial.print(ssid);
    Serial.println("...");
    WiFi.begin(ssid, pass);
    if (WiFi.waitForConnectResult() != WL_CONNECTED) return;
    Serial.println("WiFi connected");
  }
}

void connectToMQTT(){
  String clientId = "ESP8266Client-";
  clientId += String(random(0xffff), HEX);
  if (client.connect(clientId.c_str(), mqtt_user, mqtt_pass)) {
    Serial.println("Connected to MQTT server ");
    client.setServer(mqtt_server, mqtt_port);
    client.setCallback(callback);
    digitalWrite(LED_BUILTIN, 0);
    subscribeToAllTopics();
   } 
  else
    Serial.println("Could not connect to MQTT server"); 
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN,1);
  irsend.begin();
  Serial.begin(115200);
  delay(10);
  Serial.println("\n");
}

void loop() {
   connectToWiFi();
  if (WiFi.status() == WL_CONNECTED) {
    if (!client.connected()) {
      Serial.print("Connecting to MQTT server ");
      Serial.print(mqtt_server);
      Serial.println("...");
      connectToMQTT();
    }
    if (client.connected()){
      client.loop();
    }
    else
      digitalWrite(LED_BUILTIN, 1);
  }
  else
    digitalWrite(LED_BUILTIN, 1);
}