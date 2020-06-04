#include <SPI.h>
#include <Wire.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <Arduino.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <IRrecv.h>
#include <IRutils.h>

void callback(char* topic, byte* payload, unsigned int length);
unsigned long getCodeFromPayload(byte* payload, unsigned int length);
unsigned long receiveCode();
void sendCode(byte* payload, unsigned int length, String encoding);
void subscribeToAllTopics();
void connectToWiFi();
void connectToMQTT();
void handleRoot();
char* convertToCharArray(String str);
void loadPage(String page);
void wifiSetup();
void mqttSetup();
void wifiSubmit();
void mqttSubmit();
String getStringFromFile(String path);
void writeStringToFile(String str, String path);
