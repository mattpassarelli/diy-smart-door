#include <Arduino.h>
#include <EEPROM.h>
#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <Servo.h>

#include "secrets.h"

#define WIFI_SSID _WIFI_SSID  //found in secrets.h (ignored by git)
#define WIFI_PASSWORD _WIFI_PASSWORD
#define HTTP_REST_PORT 8080

ESP8266WebServer httpRestServer(HTTP_REST_PORT);
IPAddress ip(192, 168, 0, 177);
IPAddress dns(192, 168, 0, 1);
IPAddress gateway(192, 168, 0, 1);
IPAddress subnet(255, 255, 255, 0);

int servoPin = 2;
int tryCount = 0;
int openButton = 10;
int closeButton = 9;
bool isUnlocked;
Servo Servo1;

void openLock() {
    if (isUnlocked) {
        return;
    }

    int pos;
    Servo1.attach(servoPin);
    for (pos = 0; pos <= 155; pos += 1) {
        Servo1.write(pos);  // tell servo to go to position in variable 'pos'
        delay(15);          // waits 15ms for the servo to reach the position
    }
    Servo1.detach();
    isUnlocked = true;
}

void closeLock() {
    if (!isUnlocked) {
        return;
    }

    int pos;
    Servo1.attach(servoPin);

    for (pos = 155; pos >= 0; pos -= 1) {
        Servo1.write(pos);  // tell servo to go to position in variable 'pos'
        delay(15);          // waits 15ms for the servo to reach the position
    }
    Servo1.detach();

    isUnlocked = false;
}

void openLockRemotely() {
    if (isUnlocked) {
        httpRestServer.send(500, "text/html", "Deadbolt is already unlocked");
    }
    openLock();
    httpRestServer.send(200, "text/html", "Deadbolt is now unlocked");
}

void closeLockRemotely() {
    if (!isUnlocked) {
        httpRestServer.send(500, "text/html", "Deadbolt is already locked");
    }
    closeLock();
    httpRestServer.send(200, "text/html", "Deadbolt is now locked");
}

//return isUnlocked so I can see the status in HA
void getLockStatus() {
    const String status = isUnlocked ? "no" : "yes";

    httpRestServer.send(200, "text/html", "Deadbolt is locked: " + status);
}

/**
 * Ideally, I wanted to hook the ESP directly into HA
 * But I can't find a proper way to do that. ESPHome exists
 * but it doesn't seem to do what I want, so I setup a webserver
 * to handle simple REST calls to open and close the locks
 * */
void restServerRouting() {
    httpRestServer.on("/", HTTP_GET, []() {
        httpRestServer.send(200, F("text/html"),
                            F("Welcome to the Smart Door!"));
    });
    httpRestServer.on(F("/openLock"), HTTP_GET, openLockRemotely);
    httpRestServer.on(F("/closeLock"), HTTP_GET, closeLockRemotely);
    httpRestServer.on(F("/getLockStatus"), HTTP_GET, getLockStatus);
}

void setup() {
    // put your setup code here, to run once:
    Serial.begin(115200);

    Servo1.attach(servoPin);
    Servo1.write(90);
    Servo1.detach();
    pinMode(openButton, INPUT_PULLUP);
    pinMode(closeButton, INPUT_PULLUP);

    WiFi.config(ip, dns, gateway, subnet);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("Beginning WiFi Connection to: ");
    Serial.println(WIFI_SSID);

    int i = 0;
    while (WiFi.status() != WL_CONNECTED) {  // Wait for the Wi-Fi to connect
        delay(1000);
        Serial.print(++i);
        Serial.println(' ');
    }

    Serial.println('\n');
    Serial.println("Connection established!");
    Serial.print("IP address:\t");
    Serial.println(WiFi.localIP());
    WiFi.setAutoReconnect(true);

    //start http server stuff
    restServerRouting();
    httpRestServer.begin();
}

void loop() {
    //Button Detections
    if (digitalRead(openButton) == LOW) {
        Serial.println("openButton Pressed");
        openLock();
    } else if (digitalRead(closeButton) == LOW) {
        Serial.println("closeButton Pressed");
        closeLock();
    }

    //WiFi Disconnect detection
    if (WiFi.status() != WL_CONNECTED) {
        if (!isUnlocked) {
            Serial.println("WARNING: WiFi signal was lost!! Unlocking deadbolt for safety");
            //TODO: Add transistor switch
            openLock();
        }
    }

    //http server stuff
    httpRestServer.handleClient();
}
