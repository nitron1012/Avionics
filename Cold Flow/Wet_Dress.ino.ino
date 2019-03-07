#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include <Servo.h>
#include "Adafruit_MAX31855.h"

float TC [2]= {0,0};

//declare SPI pins for thermocouple breakout
int MAXDO = 50;
int TCCS [2] = {49,48};
int MAXCLK = 52;
float PT [2] = {0,0};

//initialize thermocouple breakout board
Adafruit_MAX31855 thermocouple1(MAXCLK, TCCS[0], MAXDO);
Adafruit_MAX31855 thermocouple2(MAXCLK, TCCS[1], MAXDO);

Adafruit_MAX31855 Thermocouples [2] = {thermocouple1,thermocouple2}; //create thermocouple array

//define pins for pressure transducer data
byte Pins[3] = {A0,A1};

//create a variable for the critical parameter of the oxidizer
String CRIT;
String Data = "";

//define pins, variables for oxidizer venting control
int VENT = 20;
int ventBegin;
int ventTime;

int QDS = 21;
String solenoidCondition;

////////////////////////////////////////////////


void setup() {
  //begin serial communication for debugging
  Serial.begin(115200);

  //begin communication with load cells
  Serial1.begin(115200);
  Serial1.write('c');
  delay(1);
  Serial2.begin(115200);
  Serial2.write('c');
  delay(1);

  Serial.println("STABLIZING THERMOCOUPLES...");
  // wait for MAX chip to stabilize
  delay(1000);
  Serial.println("THERMOCOUPLES STABILIZED");
  delay(1000);
  
  //initialize pressure transducers
  pinMode(Pins,INPUT);
  Serial.println("SENSORS READY");
  CRIT = String(0);

  //initialize vent timer
  pinMode(VENT,INPUT);
  ventBegin = millis();

  //initialize quick disconnect solenoid sensing
  pinMode(QDS,INPUT);
  String solenoidCondition = "OPEN";


  
  Serial.println("Initialization Complete");
  delay(1000);
}

float readWeight1()
{
    int timeOut = 2000; //Number of miliseconds before giving up
    int counter;
    
    //Clear out any trash in the buffer
    while(Serial1.available()) Serial1.read();
    
    //Send any character to trigger a read
    Serial1.print('.');
    
    //Now we need to spin to the first comma after the time stamp
    counter = 0;
    while(Serial1.read() != ',')
    {
        if(counter++ == timeOut) return(0); //Error
        delay(1);
    }
    //Now we read the weight
    counter = 0;
    String weightStr;
    while(1)  {
      if(Serial1.available()){
        char incoming = Serial1.read();
        if(incoming == ','){                
          return(weightStr.toFloat());
        }else{
          weightStr.concat(String(incoming));
        }
      }
        
      if(counter++ == timeOut) return(0); //Error
      delay(1);
  }
}      

float readWeight2()
{
    int timeOut = 2000; //Number of miliseconds before giving up
    int counter;
    
    //Clear out any trash in the buffer
    while(Serial2.available()) Serial2.read();
    
    //Send any character to trigger a read
    Serial2.print('.');
    
    //Now we need to spin to the first comma after the time stamp
    counter = 0;
    while(Serial2.read() != ',')
    {
        if(counter++ == timeOut) return(0); //Error
        delay(1);
    }
    //Now we read the weight
    counter = 0;
    String weightStr;
    while(1)  {
      if(Serial2.available()){
        char incoming = Serial2.read();
        if(incoming == ','){                
          return(weightStr.toFloat());
        }else{
          weightStr.concat(String(incoming));
        }
      }
        
      if(counter++ == timeOut) return(0); //Error
      delay(1);
  }
}      

////////////////////////////////////////////////




void loop() {
  
  Serial.println("Starting");  
  CRIT = "Stable";;
  Serial.println("Taking temperature...");
  for(int j = 0; j < 2; j++){
    TC[j] = Thermocouples[j].readCelsius(); //take temperature data in celsius
  }
  Serial.println("Taking pressure...");
  for(int k = 0; k < 2; k++){
    PT[k] = ((analogRead(Pins[k]) - 100.00) / 820.00) * 5076.00; //take pressure data from transducers, adjust for factory calibration 
  }
  
  Serial.println("Checking for critical temperature");
  for (int l = 0; l < 2; l++){ //test for critical temperature: open solenoid if above threshold, close if below
    if(TC[l] >= 309.5){
        CRIT = "Temperature";
        Serial.println("TEMPERATURE CRITICAL");
    }
  }

  Serial.println("Checking for critical pressure");
  for (int m = 0; m < 2; m++){ //test for critical pressure: open solenoid if above threshold, close if below
    if(PT[m] >= 7240){
      CRIT = "Pressure";
      Serial.println("PRESSURE CRITICAL");
    }
  }
  ////Calculating venting time////

  if(digitalRead(VENT == LOW)){
    ventBegin = millis();
    ventTime = 0;
  }else{
    ventTime = millis()-ventBegin;
  }

  if(digitalRead(QDS == LOW)){
    solenoidCondition = "CLOSED";
  }else{
    solenoidCondition = "OPEN";
  }
  
  ////Data processing////

  Data = "";
  for (int n = 0; n < 2; n++){
    Data+=TC[n];
    Data+=" ";
  }
  for (int x = 0; x < 2; x++){
    Data+=PT[x];
    Data+=" ";
  }
  
  Data+=CRIT;
  Data+=" ";
  Data+=readWeight1();
  Data+=" ";
  Data+=readWeight2();
  Data+=" ";
  Data+=ventTime;
  Data+=" ";
  Data+=solenoidCondition;
  Serial.println(Data);
}


