#include "WemosRemote.h"
#include "UserData.h"
#include "FS.h"

const uint16_t kIrLed = 12;
const uint16_t kRecvPin = 14;
const uint16_t ledPin = LED_BUILTIN;

IRsend irsend(kIrLed);
IRrecv irrecv(kRecvPin);
decode_results results;

WiFiClient wclient; 
ESP8266WebServer server(80);
PubSubClient client(mqtt_server, mqtt_port, callback, wclient);

void blinkLed(){
    for(int i=0;i<4;i++){
    digitalWrite(ledPin, 1-digitalRead(ledPin));
    delay(50);
  }
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
  blinkLed();
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
    blinkLed();
  }
}

void sendReceivedCode(byte* payload, unsigned int length){
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

void subscribeToAllTopics(){
  for(int i=0;i<(sizeof(mqtt_topics)/sizeof(*mqtt_topics));i++)
    client.subscribe(mqtt_topics[i]);
}

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
    sendReceivedCode(payload, length);
  }
}

void connectToWiFi(){
  if (WiFi.status() != WL_CONNECTED) {
    Serial.print("Connecting to ");
    Serial.print(ssid);
    Serial.println("...");
    WiFi.mode(WIFI_AP_STA);
    WiFi.begin(ssid, pass);
    WiFi.softAP("I am esp 8266!", "12345678");
    if (WiFi.waitForConnectResult() != WL_CONNECTED) return;
    Serial.println("WiFi connected");
  }
}

void connectToMQTT(){
  String clientId = "ESP8266Client-";
  clientId += String(random(0xffff), HEX);
  Serial.println("Server");
  Serial.println(mqtt_server);
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  if (client.connect(clientId.c_str(), mqtt_user, mqtt_pass)) {
    Serial.println("Connected to MQTT server ");
    digitalWrite(ledPin, 0);
    subscribeToAllTopics();
   } 
  else
    Serial.println("Could not connect to MQTT server"); 
}

String getStringFromFile(String path){
  File f = SPIFFS.open(path, "r");
  if (!f) {
      Serial.println("file open failed");
      return "";
  }
  String str = "";
  while (f.available())
    str += (char)f.read();
  return str;
}

void writeStringToFile(String str, String path){
  File f = SPIFFS.open(path, "w");
  if (!f) {
      Serial.println("file open failed");
      return;
  }
  f.print(str);
  f.close();
}

void loadPage(String page){
  File f = SPIFFS.open(page, "r");
  if (!f) {
      Serial.println("file open failed");
      return;
  }
  String index = "";
  while (f.available())
    index += (char)f.read();
  server.send(200, "text/html", index);
}

void handleRoot() {
  loadPage("/index.html");
}

char* convertToCharArray(String str){
  unsigned char* buf = new unsigned char[100];
  str.getBytes(buf, 100, 0);
  return (char*)buf;
}

void wifiSetup(){
  loadPage("/wifi.html");
}

void mqttSetup(){
  loadPage("/mqtt.html");
}

void wifiSubmit(){
  loadPage("/submit.html");
  ssid = convertToCharArray(server.arg("wifissid"));
  pass = convertToCharArray(server.arg("wifipass"));
  writeStringToFile(server.arg("wifissid"), "/wifissid.txt");
  writeStringToFile(server.arg("wifipass"), "/wifipass.txt");
  Serial.println(ssid);
  Serial.println(pass);
  WiFi.disconnect();
}

void mqttSubmit(){
  loadPage("/submit.html");
  mqtt_server = convertToCharArray(server.arg("address"));
  mqtt_user = convertToCharArray(server.arg("username"));
  mqtt_pass = convertToCharArray(server.arg("password"));
  writeStringToFile(server.arg("address"), "/mqttaddress.txt");
  writeStringToFile(server.arg("username"), "/mqttusername.txt");
  writeStringToFile(server.arg("password"), "/mqttpassword.txt");
  Serial.println(mqtt_server);
  Serial.println(mqtt_user);
  Serial.println(mqtt_pass);
  client.disconnect();
}

void setup() {
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin,1);
  irsend.begin();
  Serial.begin(115200);
  delay(10);
  Serial.println("\n");
  SPIFFS.begin();
  mqtt_server = convertToCharArray(getStringFromFile("/mqttaddress.txt"));
  mqtt_user = convertToCharArray(getStringFromFile("/mqttusername.txt"));
  mqtt_pass = convertToCharArray(getStringFromFile("/mqttpassword.txt"));
  ssid = convertToCharArray(getStringFromFile("/wifissid.txt"));
  pass = convertToCharArray(getStringFromFile("/wifipass.txt"));
  server.begin();
  server.on("/", handleRoot);
  server.on("/led", blinkLed);
  server.on("/mqtt", mqttSetup);
  server.on("/wifi", wifiSetup);
  server.on("/submit/mqtt", mqttSubmit);
  server.on("/submit/wifi", wifiSubmit);
}

void loop() {
  server.handleClient();
  connectToWiFi();
  if (WiFi.status() == WL_CONNECTED) {
    if (!client.connected()) {
      Serial.print("Connecting to MQTT server ");
      Serial.print(mqtt_server);
      Serial.println("...");
      connectToMQTT();
    }
    if (client.connected())
      client.loop();
    else
      digitalWrite(ledPin, 1);
  }
  else
    digitalWrite(ledPin, 1);
}
