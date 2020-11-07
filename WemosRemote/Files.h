#include "FS.h"

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
