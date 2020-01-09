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
//#include <ArduinoHttpClient.h>

WiFiUDP ntpUDP;

const char *ssid     = "familiaroldan";
const char *password = "51736393";

NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", 3600, 60000);

UltraSonicDistanceSensor distanceSensor(13, 12);  // Initialize sensor that uses digital pins 13 and 12.

Servo servo;

Adafruit_INA219 ina219;

const uint16_t port = 3000;
const char *host = "192.168.43.128"; //ip del router

//WebSocketClient client = WebSocketClient(wifi, serverAddress, port);

void setup(){
  uint32_t currentFrequency;
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
    servo.write(180);
    delay(1000);
    servo.write(0);
    delay(1000);
  }
  //Conversión a formato json de los datos
  DynamicJsonBuffer jsonBuffer(200);
  JsonObject &root = jsonBuffer.createObject();
  root["voltage"] = loadvoltage;
  root["current"]  = current_mA;
  root["power"]  = power_mW;
  root["state"]  = estado;
  root["level"]  = llenado;

  root.printTo(Serial);
  Serial.print("\n");
  root.printTo(client);
  client.print("\n");

  //VERIFICACION DE LA CONEXION

  //Conexion al servidor

  String url = "localhost:3000/index";

  while(client.available()){
      Serial.println("Disponible");
      String line = client.readStringUntil('\n');
      Serial.println(line);
      JsonObject& root = jsonBuffer.parseObject(line);
      if(!root.success()){
        Serial.println("parseObject() failed");
      }
      Serial.print("Requesting POST: ");
      // Send request to the server:
      client.println("POST / HTTP/1.1");
      client.println("Host: server_name");
      client.println("Accept: */*");
      client.println("Content-Type: application/x-www-form-urlencoded");
      client.print("Content-Length: ");
      client.println(line);
      client.println();
      // This will send the request to the server
      //this is a get method working
      client.print(String("GET ") + url + " HTTP/1.1\r\n" +
           "Connection: close\r\n\r\n");
      unsigned long timeout = millis();
      while (client.available() == 0) {
        if (millis() - timeout > 5000) {
          Serial.println(">>> Client Timeout !");
          client.stop();
        return;
        }
      }

  //Post de las variables
  
  }
}