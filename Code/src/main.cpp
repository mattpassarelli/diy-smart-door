#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <Servo.h>
#include <EEPROM.h>

//#define DEFAULT_WIFI_SSID "My Wifi Nework 5G"
//#define DEFAULT_WIFI_PASSWORD "MyWifiP@ssword"

int servoPin = 2;
int tryCount = 0;
int openButton = 10;
int closeButton = 9;
bool isUnlocked;
Servo Servo1;

void openLock()
{

  if (isUnlocked)
  {
    return;
  }

  int pos;
  Servo1.attach(servoPin);

  for (pos = 0; pos <= 150; pos += 1)
  { // goes from 0 degrees to 180 degrees
    // in steps of 1 degree
    Servo1.write(pos); // tell servo to go to position in variable 'pos'
    delay(15);         // waits 15ms for the servo to reach the position
  }
  Servo1.detach();
  isUnlocked = true;
}

void closeLock()
{

  if (!isUnlocked)
  {
    return;
  }

  int pos;
  Servo1.attach(servoPin);

  for (pos = 150; pos >= 0; pos -= 1)
  {                    // goes from 180 degrees to 0 degrees
    Servo1.write(pos); // tell servo to go to position in variable 'pos'
    delay(15);         // waits 15ms for the servo to reach the position
  }
  Servo1.detach();

  isUnlocked = false;
}

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(115200);

  Servo1.attach(servoPin);
  Servo1.write(90);
  Servo1.detach();
  pinMode(openButton, INPUT_PULLUP);
  pinMode(closeButton, INPUT_PULLUP);
}

void loop()
{
  // put your main code here, to run repeatedly:
  if (digitalRead(openButton) == LOW)
  {
    Serial.println("openButton Pressed");
    openLock();
  }
  else if (digitalRead(closeButton) == LOW)
  {
    Serial.println("closeButton Pressed");
    closeLock();
  }
}