#include <NX7Seg.h>


#include <ASCIIDic.h>

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>

// Pin definitions
const int latchPin = D6;
const int clockPin = D5;
const int dataPin = D7;

const char* ssid = "Your_SSID";
const char* password = "Your_PASSWORD";

const int buttonPin = D1;
const int ledPinR = D3;
const int ledPinG = D4;
const int ledPinB = D5;
const int modePin = D2;

nx7seg display(latchPin, clockPin, dataPin); // Use the correct class name
ESP8266WebServer server(80);

unsigned long pressTime = 0;
unsigned long releaseTime = 0;
unsigned long pressDuration = 0;
unsigned long bestTime = ULONG_MAX;

struct PressRecord {
  unsigned long duration;
  bool isBest;
};

PressRecord pressRecords[10];
int pressIndex = 0;

void setup() {
  Serial.begin(115200);
  pinMode(buttonPin, INPUT);
  pinMode(ledPinR, OUTPUT);
  pinMode(ledPinG, OUTPUT);
  pinMode(ledPinB, OUTPUT);
  pinMode(modePin, INPUT_PULLUP);

  // Initialize display
  display.clear(); // Use the correct function name

  // Connect to WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  server.on("/", handleRoot);
  server.on("/performance", handlePerformanceMode);
  server.begin();
}

void loop() {
  if (digitalRead(modePin) == LOW) {
    handlePerformanceMode();
  } else {
    handleCompetitionMode();
  }
  server.handleClient();
}

void handleCompetitionMode() {
  if (digitalRead(buttonPin) == HIGH) {
    if (pressTime == 0) {
      pressTime = millis();
    }
  } else {
    if (pressTime != 0) {
      releaseTime = millis();
      pressDuration = releaseTime - pressTime;
      pressTime = 0;

      updateRecords(pressDuration);
      if (pressDuration < bestTime) {
        bestTime = pressDuration;
        updateServer(bestTime);
        setLEDColor(0, 255, 255); // Turquoise
        display.writeInt(pressDuration); // Use the correct function name
      } else {
        setLEDColor(255, 165, 0); // Orange
      }
    }
  }
}

void setLEDColor(int r, int g, int b) {
  analogWrite(ledPinR, r);
  analogWrite(ledPinG, g);
  analogWrite(ledPinB, b);
}

void updateServer(unsigned long time) {
  String url = String("http://api.kits4.me/GEN/update?time=") + time;
  WiFiClient client; // Create WiFiClient object
  HTTPClient http;
  http.begin(client, url); // Use WiFiClient and URL
  int httpCode = http.GET();
  if (httpCode > 0) {
    Serial.println("Server updated successfully");
  } else {
    Serial.println("Failed to update server");
  }
  http.end();
}

void updateRecords(unsigned long time) {
  pressRecords[pressIndex] = { time, time < bestTime };
  pressIndex = (pressIndex + 1) % 10;
}

void handlePerformanceMode() {
  WiFi.softAP(ssid, password);
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);

  server.on("/", []() {
    String html = "<html><body><h1>Press Records</h1><table><tr><th>Duration (ms)</th><th>Best</th></tr>";
    for (int i = 0; i < 10; i++) {
      html += "<tr><td>" + String(pressRecords[i].duration) + "</td><td>" + (pressRecords[i].isBest ? "Yes" : "No") + "</td></tr>";
    }
    html += "</table></body></html>";
    server.send(200, "text/html", html);
  });
}

void handleRoot() {
  String message = "Welcome to the Arduino Competition!";
  server.send(200, "text/plain", message);
}

