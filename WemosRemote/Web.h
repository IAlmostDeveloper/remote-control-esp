void loadPage(String page){
  String index = getStringFromFile(page);
  server.send(200, "text/html", index);
}

char* convertToCharArray(String str){
  unsigned char* buf = new unsigned char[100];
  str.getBytes(buf, 100, 0);
  return (char*)buf;
}

void handleRoot() {
  loadPage("/index.html");
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
  mqtt_port = server.arg("port").toInt();
  mqtt_user = convertToCharArray(server.arg("username"));
  mqtt_pass = convertToCharArray(server.arg("password"));  
  writeStringToFile(server.arg("address"), "/mqttaddress.txt");
  writeStringToFile(server.arg("port"), "/mqttport.txt");
  writeStringToFile(server.arg("username"), "/mqttusername.txt");
  writeStringToFile(server.arg("password"), "/mqttpassword.txt");
  Serial.println(mqtt_server);
  Serial.println(mqtt_port);
  Serial.println(mqtt_user);
  Serial.println(mqtt_pass);
  client.disconnect();
}
