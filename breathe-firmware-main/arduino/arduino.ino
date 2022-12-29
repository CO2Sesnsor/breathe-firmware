//libraries
#include <SoftwareSerial.h>
#include <Wire.h>
#include "Adafruit_SGP30.h"
#include <SPI.h>
#include "LCD_Driver.h"
#include "GUI_Paint.h"

//definitions
#define DEBUG true
#define ESP8266_RX 2  //Connect the TX pin from the ESP to this RX pin of the Arduino
#define ESP8266_TX 3  //Connect the TX pin from the Arduino to the RX pin of ESP

SoftwareSerial esp(ESP8266_RX,ESP8266_TX);// RX, TX
const int buzzer=4;
Adafruit_SGP30 sgp;


void setup() {

pinMode(buzzer, OUTPUT); //sets pins for the buzzer
Serial.begin(9600); //beings the serial
esp.begin(9600);//default baudrate for ESP

delay(5000); //makes sure everything has started

startscreen();
startsensor();
}

//starts the screen
void startscreen()
{
  Config_Init();
  LCD_Init();
  LCD_Clear(0x000000);
  
  Paint_NewImage(LCD_WIDTH, LCD_HEIGHT, 0, BLACK);
  Paint_Clear(BLACK);
}

//resets the screen
void startsensor()
{
  if (! sgp.begin()){
 //   Serial.println("Sensor not found :(");
    while (1);
  }
  LCD_Clear(0x000000); 
}

int counter = 0;
void loop() {

if (! sgp.IAQmeasure()) {
    //Serial.println("Measurement failed");
    return;
  }

if (! sgp.IAQmeasureRaw()) {
    //Serial.println("Raw Measurement failed");
    return;
  }
//reads messages from the ESP to the serial monitor ------------
     String IncomingString="";
     boolean StringReady = false;
 
 if (esp.available()){
   IncomingString=esp.readString();
   StringReady= true;
  }
 
  if (StringReady){
    Serial.println(IncomingString);
  
  }
//-----------------------------------------------------------------------
  float voc = roundf(sgp.TVOC * 100) /100;
  float co2 = roundf(sgp.eCO2 * 100) /100;

//-----------prints the data onto the LCD screen---------------
  Paint_SetRotate(90);
  Paint_DrawString_EN(0, 0, "TVOC", &Font24, DRAW_FILL_EMPTY, RED);
  Paint_DrawFloatNum(50,40, voc,1,  &Font24, WHITE, DRAW_FILL_EMPTY);
  Paint_DrawString_EN(120, 40, "ppb", &Font24, DRAW_FILL_EMPTY, WHITE);


  Paint_DrawString_EN(0, 160, "CO2", &Font24, DRAW_FILL_EMPTY, RED);
  Paint_DrawFloatNum(50,200, co2,0,  &Font24, WHITE, DRAW_FILL_EMPTY);

  Paint_DrawString_EN(0, 80, "Raw H2", &Font24, DRAW_FILL_EMPTY, RED);
  Paint_DrawFloatNum(50,120, sgp.rawH2,0,  &Font24, WHITE, DRAW_FILL_EMPTY);

  //Paint_DrawString_EN(0, 0, "Raw Ethanol", &Font24, DRAW_FILL_EMPTY, RED);
  //Paint_DrawFloatNum(50,40, sgp.rawEthanol,0,  &Font24, WHITE, DRAW_FILL_EMPTY);

  delay(1000);
  //-------------------------------------------------------------------
  counter++;
  if (counter == 30) {
    counter = 0;

    uint16_t TVOC_base, eCO2_base;
    if (! sgp.getIAQBaseline(&eCO2_base, &TVOC_base)) { 
//      Serial.println("Failed to get baseline readings");
      return;
    }
  }
  if ((sgp.eCO2 > 1000) || (voc > 400)) //sets off the buzzer
{
  digitalWrite(buzzer, HIGH);
  delay(1000);
  digitalWrite(buzzer,LOW);
} 

  if ((sgp.eCO2 > 57000) && (sgp.TVOC > 59000)) //soft reset incase the sensor bugs
    startsensor();
}
