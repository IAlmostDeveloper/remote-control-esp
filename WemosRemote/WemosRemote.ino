#include "WemosRemote.h"

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.println("]");
  String encoding;
  String currentTopic = String(topic);
  if (currentTopic.endsWith("Controller")) {
    if (currentTopic.endsWith("/RC5Controller"))
      encoding = "RC5";
    if (currentTopic.endsWith("/NECController"))
      encoding = "NEC";
    unsigned long code = getCodeFromPayload(payload, length);
    sendCode(code, encoding);
  }
  if (currentTopic.endsWith("Controller/delay")) {
    const size_t capacity = JSON_OBJECT_SIZE(3) + JSON_ARRAY_SIZE(2) + 60;
    DynamicJsonBuffer jsonBuffer(capacity);
    JsonObject& root = jsonBuffer.parseObject(payload);
    if (!root.success()) {
      Serial.println(F("Parsing failed!"));
      return;
    }
    if (currentTopic.endsWith("/RC5Controller/delay"))
      encoding = "RC5";
    if (currentTopic.endsWith("/NECController/delay"))
      encoding = "NEC";
    String rawCode = root["code"].as<String>();
    char arr[rawCode.length() + 1];
    rawCode.toCharArray(arr, rawCode.length() + 1);
    unsigned long code = strtol(arr, NULL, 16);
    int delayTime = root["delay"].as<int>();
    Serial.println(code);
    Serial.println(delayTime);
    sendCodeWithDelay(code, encoding, delayTime);
  }
  if (currentTopic.endsWith("/receive")) {
    sendReceivedCode(payload, length);
  }
}

void setup() {
  pinMode(ledPin, OUTPUT);
  pinMode(resetPin, INPUT_PULLUP);
  digitalWrite(ledPin, 1);
  irsend.begin();
  Serial.begin(115200);
  delay(10);
  Serial.println("\n");
  SPIFFS.begin();
  mqtt_server = convertToCharArray(getStringFromFile("/mqttaddress.txt"));
  mqtt_port = getStringFromFile("/mqttport.txt").toInt();
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
unsigned long lastMsg = 0;
void loop() {
  server.handleClient();
  connectToWiFi();
  checkForReset();
  if (WiFi.status() == WL_CONNECTED) {
    if (!client.connected()) {
      Serial.print("Connecting to MQTT server ");
      Serial.print(mqtt_server);
      Serial.println("...");
      if (connectToMQTT()) {
        WiFi.mode(WIFI_STA);
        WiFi.softAPdisconnect(false);
        WiFi.enableAP(false);
        digitalWrite(ledPin, 0);
      }
    }
    if (client.connected()){
      client.loop();
      if(millis()-lastMsg>30000){
        lastMsg = millis();
        Serial.print("Publish message: alive");
        client.publish("remoteControl/devices/1/alive", "1");
      }
    }
    else
      digitalWrite(ledPin, 1);
  }
  else
    digitalWrite(ledPin, 1);
}
