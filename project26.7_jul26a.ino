#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ASCIIDic.h>

// תוכן של NX7Seg.h
#ifndef NX7Seg_h
#define NX7Seg_h

#include "Arduino.h"

class NX7Seg {
public:
    NX7Seg(int a, int b, int c, int d, int e, int f, int g, int dp);
    void begin();
    void displayNumber(int number);
private:
    int _a, _b, _c, _d, _e, _f, _g, _dp;
    void displayDigit(int digit);
};

#endif

// תוכן של NX7Seg.cpp
#include "Arduino.h"
#include "NX7Seg.h"

NX7Seg::NX7Seg(int a, int b, int c, int d, int e, int f, int g, int dp) {
    _a = a;
    _b = b;
    _c = c;
    _d = d;
    _e = e;
    _f = f;
    _g = g;
    _dp = dp;
}

void NX7Seg::begin() {
    pinMode(_a, OUTPUT);
    pinMode(_b, OUTPUT);
    pinMode(_c, OUTPUT);
    pinMode(_d, OUTPUT);
    pinMode(_e, OUTPUT);
    pinMode(_f, OUTPUT);
    pinMode(_g, OUTPUT);
    pinMode(_dp, OUTPUT);
}

void NX7Seg::displayNumber(int number) {
    // Add your implementation for displaying a number here
}

void NX7Seg::displayDigit(int digit) {
    // Add your implementation for displaying a digit here
}

const char* ssid = "Your_SSID";
const char* password = "Your_PASSWORD";

const int buttonPin = D1;
const int ledPinR = D3;
const int ledPinG = D4;
const int ledPinB = D5;
const int modePin = D2;

ESP8266WebServer server(80);
NX7Seg display(D6, D7,  D9, D10, D11, D12, D13); // עדכון לפי הפינים שבהם אתה משתמש

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

    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }
    Serial.println("Connected to WiFi");

    server.on("/", handleRoot);
    server.on("/performance", handlePerformanceMode); // כאן השינוי שבוצע
    server.begin();

    display.begin();
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
                setLEDColor(0, 255, 255); // תכלת טורקיז
            } else {
                setLEDColor(255, 165, 0); // כתום
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
    // קוד לעדכון השרת עם הזמן החדש
}

void updateRecords(unsigned long time) {
    pressRecords[pressIndex] = {time, time < bestTime};
    pressIndex = (pressIndex + 1) % 10;
}

void handlePerformanceMode() {
    WiFi.softAP(ssid, password);
    IPAddress IP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(IP);

    server.on("/performance", []() {
        String html = "<html><body><h1>Press Records</h1><table><tr><th>Duration (ms)</th><th>Best</th></tr>";
        for (int i = 0; i < 10; i++) {
            html += "<tr><td>" + String(pressRecords[i].duration) + "</td><td>" + String(pressRecords[i].isBest) + "</td></tr>";
        }
        html += "</table></body></html>";
        server.send(200, "text/html", html);
    });
}

void handleRoot() {
    String message = "Welcome to the Arduino Competition!";
    server.send(200, "text/plain", message);
}

