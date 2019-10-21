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
#include <Adafruit_INA219.h>

WiFiUDP ntpUDP;

const char *ssid     = "jose";
const char *password = "noesfake";

NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", 3600, 60000);

UltraSonicDistanceSensor distanceSensor(13, 12);  // Initialize sensor that uses digital pins 13 and 12.

Servo servo;

Adafruit_INA219 ina219;

void setup(){
  uint32_t currentFrequency;
  Serial.begin(9600);
  ina219.begin();
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

char estado;

void loop() {
  //Hora europea
  timeClient.update();
  Serial.println(timeClient.getFormattedTime());
  Serial.println("Esta es la hora gmt-0");
  //medicion de distancia
  Serial.println(distanceSensor.measureDistanceCm());
  //medicion de voltaje, corriente y potencia
  float shuntvoltage = 0;
  float busvoltage = 0;
  float current_mA = 0;
  float loadvoltage = 0;
  float power_mW = 0;

  shuntvoltage = ina219.getShuntVoltage_mV();
  busvoltage = ina219.getBusVoltage_V();
  current_mA = ina219.getCurrent_mA();
  power_mW = ina219.getPower_mW();
  loadvoltage = busvoltage + (shuntvoltage / 1000);
  
  Serial.print("Bus Voltage:   "); Serial.print(busvoltage); Serial.println(" V");
  Serial.print("Shunt Voltage: "); Serial.print(shuntvoltage); Serial.println(" mV");
  Serial.print("Load Voltage:  "); Serial.print(loadvoltage); Serial.println(" V");
  Serial.print("Current:       "); Serial.print(current_mA); Serial.println(" mA");
  Serial.print("Power:         "); Serial.print(power_mW); Serial.println(" mW");
  Serial.println("");

  //Condiconal del estado
  double llenado = distanceSensor.measureDistanceCm();
  if(llenado < 6.0){
    estado = 'F';
    Serial.println(estado);
    //accion del actuador
    servo.write(90);
    delay(1000);
    servo.write(0);
    delay(1000);
  }else{
    estado = 'N';
    Serial.println(estado);
    servo.write(0);
    delay(1000);
    servo.write(90);
    delay(1000);
  }
  //Conversión a formato json de los datos

  //Conexion a mongoDB

  //Post de las variables
  
}