//#include <ESP8266HTTPClient.h>
//#include <ESP8266WiFi.h>
//#include <WiFiClient.h>
//#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <SoftwareSerial.h>
#include <Wire.h>
#include "Adafruit_SGP30.h"

#include <SPI.h>

#include "LCD_Driver.h"
#include "GUI_Paint.h"


#define DEBUG true
#define ESP8266_RX 2  //Connect the TX pin from the ESP to this RX pin of the Arduino
#define ESP8266_TX 3  //Connect the TX pin from the Arduino to the RX pin of ESP

const int buzzer=4;
Adafruit_SGP30 sgp;

String ssid ="GUEST_WIFI";
String password="";
const char *server_url = "https://breathe-api.vercel.app/api/data";// 

StaticJsonDocument<200> jsonBuffer;
//WiFiClient client;
//HttpClient http;

SoftwareSerial esp(2,3);// RX, TX

/* return absolute humidity [mg/m^3] with approximation formula
* @param temperature [°C]
* @param humidity [%RH]
*/
uint32_t getAbsoluteHumidity(float temperature, float humidity) {
    // approximation formula from Sensirion SGP30 Driver Integration chapter 3.15
    const float absoluteHumidity = 216.7f * ((humidity / 100.0f) * 6.112f * exp((17.62f * temperature) / (243.12f + temperature)) / (273.15f + temperature)); // [g/m^3]
    const uint32_t absoluteHumidityScaled = static_cast<uint32_t>(1000.0f * absoluteHumidity); // [mg/m^3]
    return absoluteHumidityScaled;
}

void setup() {

Wire.begin();
pinMode(buzzer, OUTPUT);
//pinMode(ESP8266_RX, INPUT);
//pinMode(ESP8266_TX, OUTPUT);

esp.begin(115200);//default baudrate for ESP
//esp.listen();
Serial.begin(115200);

delay(2000);
InitWifiModule();
//resett();
//connectWifi();
startscreen();
startsensor();

  // If you have a baseline measurement from before you can assign it to start, to 'self-calibrate'
  //sgp.setIAQBaseline(0x8E68, 0x8F41);  // Will vary for each sensor!
}


void resett()
{
  esp.println("AT+RST");
  delay(2000);
  String cmd1 = "AT+CWMODE=3";
  esp.println(cmd1);
  if(esp.find("OK") ) Serial.println("Module Reset");
}

void connectWifi() 
{
  String cmd = "AT+CWJAP=\"" +ssid+"\",\"" + password + "\"";
  delay(4000);
  if(esp.find("OK")) {
  Serial.println("Connected!");
}
else {
  
connectWifi();
Serial.println("Cannot connect to wifi");
 }

}

void startscreen()
{
  Config_Init();
  LCD_Init();
  LCD_Clear(0x000000);
  
  Paint_NewImage(LCD_WIDTH, LCD_HEIGHT, 0, BLACK);
  Paint_Clear(BLACK);
}

void startsensor()
{
//  Serial.println("SGP30 test");

  if (! sgp.begin()){
 //   Serial.println("Sensor not found :(");
    while (1);
  }
  /*
  Serial.print("Found SGP30 serial #");
  Serial.print(sgp.serialnumber[0], HEX);
  Serial.print(sgp.serialnumber[1], HEX);
  Serial.println(sgp.serialnumber[2], HEX);
  */
  LCD_Clear(0x000000); 
}

int counter = 0;
void loop() {
  // If you have a temperature / humidity sensor, you can set the absolute humidity to enable the humditiy compensation for the air quality signals
  //float temperature = 22.1; // [°C]
  //float humidity = 45.2; // [%RH]
  //sgp.setHumidity(getAbsoluteHumidity(temperature, humidity));
  
/*  if(esp.available())                                           
 {    
    if(esp.find("+IPD,"))
    {
     delay(1000);
 
     int connectionId = esp.read()-48;                                                
     String webpage = "<h1>Hello World!</h1>";
     String cipSend = "AT+CIPSEND=";
     cipSend += connectionId;
     cipSend += ",";
     cipSend +=webpage.length();
     cipSend +="\r\n";
     
     sendData(cipSend,1000,DEBUG);
     sendData(webpage,1000,DEBUG);
 
     String closeCommand = "AT+CIPCLOSE="; 
     closeCommand+=connectionId; // append connection id
     closeCommand+="\r\n";    
     sendData(closeCommand,3000,DEBUG);
    }
  }*/

  if (! sgp.IAQmeasure()) {
    //Serial.println("Measurement failed");
    return;
  }
  float voc = roundf(sgp.TVOC * 100) /100;
  float co2 = roundf(sgp.eCO2 * 100) /100;
  Serial.print("eCO2 "); Serial.print(sgp.eCO2);
  Serial.print(" TVOC "); Serial.print(sgp.TVOC); //Serial.print(" ppb\t");
  Serial.println("");

  if (! sgp.IAQmeasureRaw()) {
   // Serial.println("Raw Measurement failed");
    return;
  }

//  Serial.print("Raw H2 "); Serial.print(sgp.rawH2); Serial.print(" \t");
//  Serial.print("Raw Ethanol "); Serial.print(sgp.rawEthanol); Serial.println("");


  JsonObject values = jsonBuffer.to<JsonObject>();
  values["co2"] = co2;
  values["voc"] = voc;

/*
http.begin(client, server_url);
    http.addHeader("Content-Type", "application/json");
    int httpCode = http.POST(values);
    if(httpCode > 0){
      if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
          String payload = http.getString();
          Serial.print("Response: ");Serial.println(payload);
        }
    }else{
         Serial.printf("[HTTP] GET... failed, error: %s", http.errorToString(httpCode).c_str());
    }
    http.end();
*/
  Paint_DrawString_EN(0, 240, "TVOC", &Font24, DRAW_FILL_EMPTY, RED);
  Paint_DrawFloatNum(50,280, voc,1,  &Font24, WHITE, DRAW_FILL_EMPTY);

  Paint_DrawString_EN(0, 160, "CO2", &Font24, DRAW_FILL_EMPTY, RED);
  Paint_DrawFloatNum(50,200, co2,0,  &Font24, WHITE, DRAW_FILL_EMPTY);

  Paint_DrawString_EN(0, 80, "Raw H2", &Font24, DRAW_FILL_EMPTY, RED);
  Paint_DrawFloatNum(50,120, sgp.rawH2,0,  &Font24, WHITE, DRAW_FILL_EMPTY);

  Paint_DrawString_EN(0, 0, "Raw Ethanol", &Font24, DRAW_FILL_EMPTY, RED);
  Paint_DrawFloatNum(50,40, sgp.rawEthanol,0,  &Font24, WHITE, DRAW_FILL_EMPTY);

  delay(1000);
  
  counter++;
  if (counter == 30) {
    counter = 0;

    uint16_t TVOC_base, eCO2_base;
    if (! sgp.getIAQBaseline(&eCO2_base, &TVOC_base)) { 
//      Serial.println("Failed to get baseline readings");
      return;
    }
//   Serial.print("****Baseline values: eCO2: 0x"); Serial.print(eCO2_base, HEX);
//   Serial.print(" & TVOC: 0x"); Serial.println(TVOC_base, HEX);
  }

  if ((sgp.eCO2 > 1000) || (voc > 400))
{
  digitalWrite(buzzer, HIGH);
  delay(1000);
  digitalWrite(buzzer,LOW);
} 

  if ((sgp.eCO2 > 57000) && (sgp.TVOC > 59000))
    startsensor();
}

//sending the data to esp
String sendData(String command, const int timeout, boolean debug)
{
    String response = "";                                             
    esp.print(command);                                          
    long int time = millis();                                      
    while( (time+timeout) > millis())                                 
    {      
      while(esp.available())                                      
      {
        char c = esp.read();                                     
        response+=c;                                                  
      }  
    }    
    if(debug)                                                        
    {
      Serial.print(response);
    }    
    return response;                                                  
}
//AT commands for the esp to connect to the WiFi
void InitWifiModule()
{
  sendData("AT+RST\r\n", 2000, DEBUG);                                                  
  sendData("AT+CWJAP=\"GUEST_WIFI\",\"\"\r\n", 2000, DEBUG);        
  delay (3000);
  sendData("AT+CWMODE=1\r\n", 1500, DEBUG);                                             
  delay (1500);
  sendData("AT+CIFSR\r\n", 1500, DEBUG);                                             
  delay (1500);
  sendData("AT+CIPMUX=1\r\n", 1500, DEBUG);                                             
  delay (1500);
  sendData("AT+CIPSERVER=1,80\r\n", 1500, DEBUG);                                     

}
