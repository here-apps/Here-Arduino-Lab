#include <SoftwareSerial.h>       
#include <avr/sleep.h>            //Arduino Sleep Mode
#include <avr/wdt.h>              //Arduino WatchDog Timer setting : related to sleep mode

SoftwareSerial BTSerial(10, 11);  // SoftwareSerial RX | TX for bluetooth

const int ledpin=13;              //LED pin
const int buzzerpin=7;            //Buzzer pin
const int buttonpin=3;            //flame sensor pin

int flameval;                     //flame flag (fire or not)
int gasreading;                   

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
  pinMode(ledpin,OUTPUT);              //LED pin as OUTPUT
  pinMode(buzzerpin,OUTPUT);           //BUZZER pin as OUTPUT
  pinMode(buttonpin,INPUT);            //FLAME SENSOR pin as INPUT
  
  BTSerial.begin(9600);
  Serial.begin(57600); 
  Serial.println("Start detecting...");
}
void loop()
{
    //variable declaration   
    String outputData="";     
    char tmp[10];
    boolean readFailFlag=0;     
    
    flameval=digitalRead(buttonpin);    //read flame sensor pin and assign to flameval
    if(flameval==HIGH)                  //check fire
    {      
    //reading sensing data  
      gasreading  = map(analogRead(A1), 0,1023,300,10000);   // gas concentration, map 0~1023 to 300~10000ppm
      
      if(isnan(gasreading))                                     //  Determine if sensor readings are error.     
        Serial.println("Failed to read from sensors!!");
      digitalWrite(ledpin,HIGH);
      digitalWrite(buzzerpin,HIGH);
      
      Serial.print("Fire!!!  ");
      Serial.println(analogRead(A0));       
           
      outputData = outputData+flameval+","+gasreading;
   
  /*---------Bluetooth transfer-----------*/    
      BTSerial.println(outputData);      
    }else{ 
      digitalWrite(ledpin,LOW);
      digitalWrite(buzzerpin,LOW);     
    }
/*---------Sleep mode------------------*/      
    myWatchdogEnable (0b100001);
}
