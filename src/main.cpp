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
#include <ArduinoJson.h>
#include <ArduinoHttpClient.h>
#include <stdio.h>
#include <stdlib.h>

WiFiUDP ntpUDP;

const char *ssid     = "Jose";//Usergioarboleda Jose
const char *password = "noesfake";//ceskqyw2012 noesfake

NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", 3600, 60000);

UltraSonicDistanceSensor distanceSensor(13, 12);  // Initialize sensor that uses digital pins 13 and 12.

Servo servo;

Adafruit_INA219 ina219;

const char *host = "192.168.43.25"; //ip del router 192.168.43.25 / 172.25.8.193
const uint16_t port = 8081;

int status=0;

//WebSocketClient client = WebSocketClient(wifi, serverAddress, port);

void setup(){
  Serial.println(timeClient.getFormattedTime());
  Serial.begin(9600);
  ina219.begin();
  pinMode(2, OUTPUT);
  servo.attach(2);
  servo.write(0);
  WiFi.begin(ssid, password);

  while ( WiFi.status() != WL_CONNECTED ) {
    delay ( 500 );
    Serial.print ( "." );
  }

  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
  delay(1000);
}

String estado;
WiFiClient client;
HttpClient http = HttpClient(client, host, port);
String tiempo = timeClient.getFormattedTime();

void loop() {
  //medicion de distancia
  Serial.print("Distancia: ");
  Serial.print(distanceSensor.measureDistanceCm()); Serial.println(" cm");
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

  double llenado = distanceSensor.measureDistanceCm();
  //Conversión a formato json de los datos
  DynamicJsonBuffer jsonBuffer(200);
  JsonObject &root = jsonBuffer.createObject();
  root["voltage"] = loadvoltage;
  root["current"]  = current_mA;
  root["power"]  = power_mW;
  root["state"]  = estado;
  root["level"]  = llenado;
  root["time"] = tiempo;

  root.printTo(Serial);
  Serial.print("\n");

  String json_test = "{\"voltage\":10,\"current\":\"1\",\"power\":\"6\",\"state\":\"f\",\"level\":\"6\"}";

  //VERIFICACION DE LA CONEXION

  client.connect(host, port);
  client.print("Succesfully connected to host");

  // We now create a URI for the request
  String url = "http://localhost:8081/";
  String unlock_url = "http://localhost:8081/unlock";

  // Send request to the server:
  client.println("POST / HTTP/1.1");
  client.println("Host: localhost");
  client.println("Accept: */*");
  client.println("Content-Type: application/json");
  //client.print("Content-Length: ");
  //client.println(data.length());
  client.println();
  root.printTo(client);
  client.print("\n");
  //convert to char
  char json_conv[100];
  root.printTo((char*)json_conv, root.measureLength());
  //convert to string
  String json_str;
  root.printTo(json_str);
  Serial.println(json_str);
  client.println(json_str);

  /*
  //this is a get method working
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
           "Connection: close\r\n\r\n");
  */

  //POST REQUEST
  http.beginRequest();
  http.post(url, "application/json", json_str);//DATA MUST BE STRING, SEND DATA TO SERVER
  http.endRequest();

  //GET REQUEST
  http.beginRequest();
  http.get(unlock_url);//OBTAIN UNLOCK DATA FROM SERVER
  http.endRequest();
 
  unsigned long timeout = millis();
  while (client.available() == 0) {
    if (millis() - timeout > 5000) {
      Serial.println(">>> Client Timeout !");
      client.println(">>> Client Timeout !");
      client.stop();
      return;
    }
  }

  // Read all the lines of the reply from server and print them to Serial
  while(client.available()){
    String dato_bloqueo = client.readStringUntil('\r');//STRINGYFY DATA
    estado = dato_bloqueo;
    Serial.println(estado);
  }

  //asign to switch int variable
  if(((estado.compareTo("\nF")==0)) && (llenado < 6.0)){
    status = 1;
  }else if((estado.compareTo("\nN")==0)){
    status = 2;
  }

  Serial.println(status);

  switch(status){
    case 0:
    Serial.println("No response case 0");
    break;
    case 1:
      Serial.println(estado);
      Serial.println("FULL 1");
      for ( int pos = 0; pos <= 180; pos += 1) { // goes from 0 degrees to 180 degrees
      // in steps of 1 degree
      servo.write(pos);              // tell servo to go to position in variable 'pos'
      delay(15);
      }                     
    break;
    case 2:
      Serial.println(estado);
      Serial.println("NORMAL 2");
      for ( int pos = 180; pos >= 0; pos -= 1) { // goes from 180 degrees to 0 degrees
      servo.write(pos);              // tell servo to go to position in variable 'pos'
      delay(15);                       // waits 15ms for the servo to reach the position
      }                    
    break;
    default:
    Serial.println("No response case default");
  }

  /*
  //CONDICIONAL DE LLENADO
  if((llenado < 6.0) && ((estado.compareTo("F")==0)==0)){
    Serial.println("FULL");
    for ( int pos = 0; pos <= 180; pos += 1) { // goes from 0 degrees to 180 degrees
    // in steps of 1 degree
    servo.write(pos);              // tell servo to go to position in variable 'pos'
    delay(15);
    }                     
  }
  else if((estado.compareTo("N")==0)==0){
      Serial.println("NORMAL");
      for ( int pos = 180; pos >= 0; pos -= 1) { // goes from 180 degrees to 0 degrees
      servo.write(pos);              // tell servo to go to position in variable 'pos'
      delay(15);                       // waits 15ms for the servo to reach the position
    }
  }
  */ 
      

  Serial.println();
  Serial.println("closing connection");

}