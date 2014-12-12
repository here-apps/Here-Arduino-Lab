#include <PinChangeInt.h>
#include <eHealth.h>
#include <stdio.h>


const byte MSG_CMD=0x00;        // Bluetooth transmit command, 0x00 message
const byte DATA_CMD=0x01;       // 0x01 Data
const byte FILE_SEND_CMD=0x02;  // 0x02 File Content

const int updateBtn = 4;     // button pin
int buttonState = 0;         // variable for reading the pushbutton status 

void setup() { 
  eHealth.readBloodPressureSensor();  
  Serial.begin(9600);        // Serial initialization, set baud rate, 9600 is the default baud rate of HC-06
  delay(100);    
}

void loop() { 
  String s="Waiting";
  String outputData="";
  char tmp[10];

  uint8_t numberOfData = eHealth.getBloodPressureLength();   

  int i = 0;
  if(numberOfData>0)
  {
    i = numberOfData-1;
  }
  else
  {
    i = 0;
  }

  int systoli=30+eHealth.bloodPressureDataVector[i].systolic;
  int diastolic=eHealth.bloodPressureDataVector[i].diastolic;
  int pulse=eHealth.bloodPressureDataVector[i].pulse;
  outputData=outputData+systoli+","+diastolic+","+pulse+",";

  float conductance = eHealth.getSkinConductance();
  dtostrf(conductance,2,2,tmp);
  outputData=outputData+tmp+",";
  float resistance = eHealth.getSkinResistance();
  dtostrf(resistance,2,2,tmp);
  outputData=outputData+tmp+","; 
  float conductanceVol = eHealth.getSkinConductanceVoltage();
  dtostrf(conductanceVol,2,2,tmp);
  outputData=outputData+tmp;
  buttonState = digitalRead(updateBtn);    

  if(buttonState)
  {
    Serial.write(DATA_CMD);                      // BT transmit command
    Serial.write(outputData.length());           // data length
    Serial.println(outputData);                  // transmit data to device
    delay(500);         
  }
  delay(2000);
}
