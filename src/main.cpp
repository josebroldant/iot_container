#include <Arduino.h>
#include <NTPClient.h>
// change next line to use with another board/shield
#include <ESP8266WiFi.h>
//#include <WiFi.h> // for WiFi shield
//#include <WiFi101.h> // for WiFi 101 shield or MKR1000
#include <WiFiUdp.h>
#include <Wire.h>
#include <HCSR04.h>
#include <Servo.h>

WiFiUDP ntpUDP;

const char *ssid     = "jose";
const char *password = "noesfake";

NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", 3600, 60000);

UltraSonicDistanceSensor distanceSensor(13, 12);  // Initialize sensor that uses digital pins 13 and 12.

Servo servo;

void setup(){
  Serial.begin(9600);

  WiFi.begin(ssid, password);

  while ( WiFi.status() != WL_CONNECTED ) {
    delay ( 500 );
    Serial.print ( "." );
  }

  timeClient.begin();
  pinMode(2, OUTPUT);
  servo.attach(2);
  servo.write(0);
  delay(1000);
}

void loop() {

  //Aqui en el loop hacer el post constante de las variables.
  timeClient.update();

  Serial.println(timeClient.getFormattedTime());
  Serial.println("Esta es la hora gmt-0");
  Serial.println(distanceSensor.measureDistanceCm());
  servo.write(90);
  delay(1000);
  servo.write(0);
  delay(1000);
}