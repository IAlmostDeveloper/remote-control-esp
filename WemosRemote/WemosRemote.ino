#include "WemosRemote.h"

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

void setup() {
  pinMode(ledPin, OUTPUT);
  pinMode(resetPin, INPUT_PULLUP);
  digitalWrite(ledPin,1);
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

void loop() {
  server.handleClient();
  connectToWiFi();
  checkForReset();
  if (WiFi.status() == WL_CONNECTED) {
    if (!client.connected()) {
      Serial.print("Connecting to MQTT server ");
      Serial.print(mqtt_server);
      Serial.println("...");
      if(connectToMQTT()) {
        WiFi.mode(WIFI_STA);
        WiFi.softAPdisconnect(false);
        WiFi.enableAP(false);
        digitalWrite(ledPin, 0);
        }
    }
    if (client.connected())
      client.loop();
    else
      digitalWrite(ledPin, 1);
  }
  else
    digitalWrite(ledPin, 1);
}
