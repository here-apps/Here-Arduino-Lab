//Project Name: Indoor Green Monitoring Station

#include <SoftwareSerial.h>       

#include "DHT.h"                  //DHT Humidity&Temperature 
#include <avr/sleep.h>            //Arduino Sleep Mode
#include <avr/wdt.h>              //Arduino WatchDog Timer setting : related to sleep mode

#define DHTPIN A0                 // what pin we're connected to get Humidity&Temperature

// Uncomment whatever type of humidity&temperature module you're using!
//#define DHTTYPE DHT11           // DHT 11 
#define DHTTYPE DHT22             // DHT 22  (AM2302)  
//#define DHTTYPE DHT21           // DHT 21 (AM2301)

SoftwareSerial BTSerial(10, 11);  // SoftwareSerial RX | TX for bluetooth

const int timeCnt= 45;            //timeCnt*8sec: total sleeping time
const int moisturePin=A1;         //Soil moisture pin
const int NumOfSensorReading =3;  //no. of sensor readings
const float MAX_MOISTURE=950;     //max soil moisture (soilreading/max==>%)

DHT dht(DHTPIN, DHTTYPE);

/*======================Sleep mode with WatchDog Timer=========================*/
// watchdog interrupt
ISR(WDT_vect) 
  {
  wdt_disable();  // disable watchdog
  }

void myWatchdogEnable(const byte interval) 
  {  
  MCUSR = 0;                          // reset various flags
  WDTCSR |= 0b00011000;               // see docs, set WDCE, WDE
  WDTCSR =  0b01000000 | interval;    // set WDIE, and appropriate delay

  wdt_reset();
  set_sleep_mode (SLEEP_MODE_PWR_DOWN);  
  sleep_mode();            // now goes to Sleep and waits for the interrupt
  } 
//==================================setup()====================================  
void setup() 
{
    Serial.begin(57600);                                      //  Set the baud rate of the Serial Monitor
    BTSerial.begin(9600);                                    //  Setting the baud rate of Bluetooth module
    dht.begin();
}
//==================================loop()=====================================  
void loop() 
{
    //variable declaration   
    String outputData="";     
    float sensedData[3];      // 0: dht humidity, 1: dht temperature, 2: soil moisture
    char tmp[10];
    boolean readFailFlag=0;   
    
    //Reading Sensing data
    sensedData[0]  = analogRead(moisturePin)/MAX_MOISTURE;   //  Soil Moisture (%)
    sensedData[1]  = dht.readTemperature();                  //  Temperature  (℃) 
    sensedData[2] =  dht.readHumidity();                     //  Humidity      (%)
    
    for(int i=0;i<NumOfSensorReading;i++)                    //  Determine if sensor readings are error.
      readFailFlag = readFailFlag || isnan(sensedData[i]);   //  readFailFlag is set if error
    
    if(readFailFlag)                                          
      Serial.println("Failed to read from sensors!!");
    
    for(int i=0;i<NumOfSensorReading;i++)                    //  Combine all sensor readings into a single string
    {
      dtostrf(sensedData[i],2,2,tmp);                        
      outputData = outputData+tmp;
      if(i!=NumOfSensorReading-1)      
        outputData+=",";        
    }

    char outData[outputData.length()];                        //  Print sensor reading string on Serial Monitor
//    Serial.println(outputData); 

/*---------Bluetooth transfer-----------*/    
    BTSerial.println(outputData);
/*---------Sleep mode------------------*/
    for(int i=0;i<timeCnt;i++)
    {
      myWatchdogEnable (0b100001);
    }
}

// sleep bit patterns:
//  1 second:  0b000110
//  2 seconds: 0b000111
//  4 seconds: 0b100000
//  8 seconds: 0b100001
