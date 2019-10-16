#include <NTPClient.h>
// change next line to use with another board/shield
#include <ESP8266WiFi.h>
//#include <WiFi.h> // for WiFi shield
//#include <WiFi101.h> // for WiFi 101 shield or MKR1000
#include <WiFiUdp.h>

#include <HCSR04.h>

UltraSonicDistanceSensor distanceSensor(13, 12);  // Initialize sensor that uses digital pins 13 and 12.


WiFiUDP ntpUDP;

const char *ssid     = "jose";
const char *password = "<1234567890>";

NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", 3600, 60000);

void setup(){
  Serial.begin(115200);

  WiFi.begin(ssid, password);

  while ( WiFi.status() != WL_CONNECTED ) {
    delay ( 500 );
    Serial.print ( "." );
  }

  timeClient.begin();
}

void loop() {

  //Aqui en el loop hacer el post constante de las variables.
  timeClient.update();

  Serial.println(timeClient.getFormattedTime());

  Serial.println(distanceSensor.measureDistanceCm());

  delay(1000);

  
}