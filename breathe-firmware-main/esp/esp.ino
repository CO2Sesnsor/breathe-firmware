//libraries
#include <Wire.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include "Adafruit_SGP30.h"

Adafruit_SGP30 sgp;
//eCO2_baseline = 400;
//TVOC_baseline = 0;

//ssid, password and the URL of the server
const char *ssid = "GUEST_WIFI"; //Enter your WIFI ssid
const char *password = ""; //Enter your WIFI password
const char *server_url = "https://breathe-api.vercel.app/api/data";// Nodejs application endpoint

//object
WiFiClient client;
HTTPClient http;
String json;

void setup() {
  Wire.begin(2,0); //rewires the SDA and SCL (for I2C com) to GPI2 and GPIO
   delay(3000);
   Serial.begin(9600); //begins serial at 9600 baudrate
   sgp.begin();
   WiFi.begin(ssid, password); //begins wifi connection
   while (WiFi.status() != WL_CONNECTED) { //checks if wifi is connected
    delay(500);
    Serial.print(".");
   }
   Serial.println("WiFi connected");
   delay(1000);
}
int counter = 0;
void loop() {

 if (! sgp.IAQmeasure()) {
    //Serial.println("Measurement failed");
    return;
  }
  
  float voc = roundf(sgp.TVOC * 100) /100;
  float co2 = roundf(sgp.eCO2 * 100) /100;

 if (! sgp.IAQmeasureRaw()) {
    //Serial.println("Raw Measurement failed");
    return;
  }

//initialising Json
StaticJsonBuffer<512> jsonBuffer; //reserve memory
JsonObject& values = jsonBuffer.createObject();
  values["co2"] = co2;
  values["voc"] = voc;

  values.printTo(Serial); //prints the values to the serial monitor 
  values.prettyPrintTo(json); //converts the values to a json string so it can be posted
  checkserver(); //HTTP POST function
  delay(5000);

counter++;
  if (counter == 30) {
    counter = 0;

    uint16_t TVOC_base, eCO2_base;
    if (! sgp.getIAQBaseline(&eCO2_base, &TVOC_base)) { 
//      Serial.println("Failed to get baseline readings");
      return;
    }
}
}

void checkserver(){

    HTTPClient http;
    http.begin(client, server_url); //domain name w/ URL path
    http.addHeader("Content-Type", "application/json"); //content-type header for requesting to the server
    int httpCode = http.POST(json); //sending json values with HTTP POST
    http.end(); //ending POST
    Serial.println("");
    Serial.print("Response:");
    Serial.print(httpCode); //tells whether it was successful
    Serial.println("");
}
