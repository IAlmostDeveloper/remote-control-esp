WiFiClient wclient; 
ESP8266WebServer server(80);
void callback(char* topic, byte* payload, unsigned int length);
PubSubClient client(mqtt_server, mqtt_port, callback, wclient);

void subscribeToAllTopics(){
  for(int i=0;i<(sizeof(mqtt_topics)/sizeof(*mqtt_topics));i++)
    client.subscribe(mqtt_topics[i]);
}

bool connectToWiFi(){
  String apSsid = "RemoteControl-";
  apSsid += 1;
  if (WiFi.status() != WL_CONNECTED) {
    Serial.print("Connecting to ");
    Serial.print(ssid);
    Serial.println("...");
    WiFi.mode(WIFI_AP_STA);
    WiFi.begin(ssid, pass);
    WiFi.softAP(apSsid, "12345678");
    if (WiFi.waitForConnectResult() != WL_CONNECTED) return false;
    Serial.println("WiFi connected");
    return true;
  }
  return false;
}

bool connectToMQTT(){
  String clientId = "ESP8266Client-";
  clientId += String(random(0xffff), HEX);
  Serial.println("Server");
  Serial.println(mqtt_server);
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  if (client.connect(clientId.c_str(), mqtt_user, mqtt_pass)) {
    Serial.println("Connected to MQTT server ");
    
    subscribeToAllTopics();
    return true;
   } 
  else{
    Serial.println("Could not connect to MQTT server"); 
    return false;
    }
}
