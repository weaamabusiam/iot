
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <WiFiUdp.h>
#include <ESP8266HTTPClient.h>


const char* ssid = "Kinneret College";
const char* password = "55555333";
WiFiClient client;
int server_port = 80; 



void wifiClient_Setup() {
  Serial.println("wifiSetup");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.println("trying ...");
    delay(100); 
  }
  Serial.println("Connected to network");
}



void SendData(float PressDurationSeconds) {
  HTTPClient http;
  String dataURL = "http://api.kits4.me/GEN/api.php?"; 
  dataURL += "ACT=SET&DEV=1121&CH=1&VAL=";
  dataURL += String(PressDurationSeconds);

  http.begin(client, dataURL);
  int httpCode = http.GET();
  Serial.println(httpCode);
  http.end();
}


int GetData() {
  int ret = -1;
  HTTPClient http;
  String dataURL = "http://api.kits4.me/GEN/api.php?";  
  dataURL += "ACT=GET&DEV=1121&CH=1";

  http.begin(client, dataURL);
  int httpCode = http.GET();
  Serial.println(httpCode);
  delay(1000);

  if (httpCode == HTTP_CODE_OK) {
    Serial.print("HTTP response code ");
    Serial.println(httpCode);
    String Res = http.getString();
    Serial.println(Res);
    ret = Res.toInt();
  }
  
  http.end();
  return ret;
}




void setup() {
  Serial.begin(115200);
  wifiClient_Setup();
}

void loop() {


  SendData(1.23); 
  int receivedData = GetData();
  delay(5000);
}
