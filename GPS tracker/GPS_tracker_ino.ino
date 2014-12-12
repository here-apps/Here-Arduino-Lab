#include <TinyGPS++.h>
#include <SoftwareSerial.h>
#include <SPI.h>
#include <SD.h>

const int buttonPin = 6;    // the number of the pushbutton pin
const int ledPin = 9;       // the number of the LED pin

// Debouncing detection
int buttonState;             // the current reading from the input pin
int lastButtonState = LOW;   // the previous reading from the input pin
long lastDebounceTime = 0;   // the last time the output pin was toggled
long debounceDelay = 50;     // the debounce time; increase if the output flickers
long longPressTime = 5000;   // duration of button is hold at "pressed" state
long currentTime;          

TinyGPSPlus gps;              // The TinyGPS++ object
unsigned char s;
const int logfrequency = 8000;  //frequency of log GPS data 

SoftwareSerial GPSserial(2, 3);  //SoftwareSerial for GPS 

File myFile;                     //GPS log file in the SD 
int closedflag=0;                //check if file is closed 

// Bluetooth Content Command, file content==FILE_SEND_CMD
const byte MSG_CMD=0x00;          //general string (may bring file info)
const byte FILE_SEND_CMD=0x01;    //file content
const int PIC_PKT_LEN = 128;      //buffer size transferred

void setup() {  
  Serial.begin(57600);             //Serial baud rate (bluetooth)
  GPSserial.begin(9600);          //GPS baud rate 
  pinMode(buttonPin, INPUT);      //Set buttonPin as INPUT
  pinMode(ledPin, OUTPUT);        //Set ledPin as OUTPUT

  // set initial LED state
  digitalWrite(ledPin, LOW);

  if (!SD.begin(4)) {              //SD module initialization, CS=4
    return;
  }

  myFile = SD.open("tracker.txt", FILE_WRITE);  //open GPS log file for writing...
  if(myFile)                                    //check if file exists
  {
    closedflag=1;
  }
  else                                          
    return;

}

void loop() {
  long tmp;
  int reading = digitalRead(buttonPin);         

  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  } 

  if ((millis() - lastDebounceTime) > debounceDelay) {    //check Debounce delay

    if(reading)                                            
    {   
      tmp= millis()-currentTime;
      if(tmp>=longPressTime)                               //if pushbutton is hold at "pressed" state more than longPressTime,
      {                                                    //then...
        digitalWrite(ledPin, HIGH);                        //LED light
        if(closedflag=1)        
        {
          myFile.close();                                  //close file
          closedflag=0;
        }        

        SendToBT();                                        //transfer file through Bluetooth
        digitalWrite(ledPin, LOW);                         //after done transfer, LED OFF
        return;                                            //the program returns here, reset arduino to restart
      }
    }else
      currentTime=millis();
    
    // if the button state has changed:  HIGH->LOW or LOW->HIGH
    if (reading != buttonState) {
      buttonState = reading;
    }
  }
    
  lastButtonState = reading;

  while (GPSserial.available() > 0)                          //Reading GPS data
  {
    s=GPSserial.read();
    if (gps.encode(s))                                        //TinyGPS++ parsing data
      displayInfo();                                          
  }
  if (millis() > 5000 && gps.charsProcessed() < 10)          
  {
    while(true);
  }    
}

void displayInfo()
{
  int flag=0;
      
  if (gps.location.isValid())
    flag++;

  if (gps.date.isValid())
    flag++;

  if (gps.time.isValid())
    flag++;

  if(gps.altitude.isValid())
    flag++;

  if(flag==4)                      //only when we get location, date, time, and altitude data,
  {                                //we log gps data
      myFile.print(gps.location.lat(), 6);
      myFile.print(F(","));
      myFile.print(gps.location.lng(), 6);    
      myFile.print(";");
      myFile.print(gps.date.year());
      myFile.print("-");    
      if(gps.date.month()<10)  myFile.print(F("0"));
      myFile.print(gps.date.month());
      myFile.print("-");    
      if(gps.date.day()<10)  myFile.print(F("0"));
      myFile.print(gps.date.day());
      myFile.print("T");
      if (gps.time.hour() < 10) myFile.print(F("0"));
      myFile.print(gps.time.hour());
      myFile.print(F(":"));
      if (gps.time.minute() < 10) myFile.print(F("0"));
      myFile.print(gps.time.minute());
      myFile.print(F(":"));
      if (gps.time.second() < 10) myFile.print(F("0"));
      myFile.print(gps.time.second());
      myFile.print(F("."));
      if (gps.time.centisecond() < 10) myFile.print(F("0"));
      myFile.print(gps.time.centisecond());
      myFile.print("Z");
      myFile.print(";");
      myFile.println(gps.altitude.meters(),7);          
      delay(logfrequency);            //log frequency
  }
  myFile.flush();
}


void SendToBT()                        //file transfer function
{
  String s="";
  int cnt, j, k;
  char picN[]="tracker.txt";

  myFile=SD.open(picN);
  if(myFile)
  {
  }else
  {
    return;
  }
  
  s = s+picN+","+myFile.size();

  //file info transferring through Bluetooth (filename, filesize)
  Serial.write(MSG_CMD);  
  Serial.write(s.length());
  Serial.print(s); 
  
  delay(500);
  
  byte wrbuffer[PIC_PKT_LEN];
  cnt = myFile.size();
   
  int tmp, tmp2;

  if(Serial.available())
  {
    delay(500);
    while(myFile.available())
    { 
        tmp = cnt/PIC_PKT_LEN;
        if(tmp>0)
        { 
          for(j=0;j<PIC_PKT_LEN;j++)  
          {   
            wrbuffer[j]=myFile.read();
          }
          cnt-= PIC_PKT_LEN;
          //file content transferring through Bluetooth
          Serial.write(FILE_SEND_CMD); 
          Serial.write(PIC_PKT_LEN);
          Serial.write(wrbuffer, PIC_PKT_LEN);  //Send by bt
        }else
        {
          tmp2 = cnt%PIC_PKT_LEN;
          if(tmp2!=0)
          {            
            for(j=0;j<tmp2;j++)
            {     
              wrbuffer[j]=myFile.read();
            }
          //file content transferring through Bluetooth            
            Serial.write(FILE_SEND_CMD); 
            Serial.write(tmp2);            
            Serial.write(wrbuffer, tmp2);
          }
        }                 
    } 
  }
  myFile.close();    
}
