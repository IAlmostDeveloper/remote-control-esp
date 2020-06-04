const uint16_t kIrLed = 12;
const uint16_t kRecvPin = 14;
const uint16_t ledPin = LED_BUILTIN;

IRsend irsend(kIrLed);
IRrecv irrecv(kRecvPin);
decode_results results;


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
