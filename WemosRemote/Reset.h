const uint16_t resetPin = 13;
bool pressed = false;
unsigned long btnTimer = 0;
int timeForHardReset = 3000;
String settingsFiles[] = {"/mqttaddress.txt", "/mqttport.txt", "/mqttusername.txt", "/mqttpassword.txt", "/wifissid.txt", "/wifipass.txt"};

void clearSettings(){
  for (String fileName : settingsFiles){
    File f = SPIFFS.open(fileName, "w");
    f.close();
  }
}

void hardReset(){
  clearSettings();
  ESP.reset();
}

void softReset(){
  ESP.reset();
}

void checkForReset(){
  btnTimer = millis();
  pressed = !digitalRead(resetPin);
  while(pressed){
    pressed = !digitalRead(resetPin);
    if(millis()-btnTimer>timeForHardReset){
      hardReset();
      return;
    }
    else if(!pressed){
      softReset();
      return;
    }
    delay(5);
  }
}

