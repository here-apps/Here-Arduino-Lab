/*
 *  eHealth sensor platform for Arduino and Raspberry from Cooking-hacks.
 *
 *  Description: "The e-Health Sensor Shield allows Arduino and Raspberry Pi 
 *  users to perform biometric and medical applications by using 9 different 
 *  sensors: Pulse and Oxygen in Blood Sensor (SPO2), Airflow Sensor (Breathing),
 *  Body Temperature, Electrocardiogram Sensor (ECG), Glucometer, Galvanic Skin
 *  Response Sensor (GSR - Sweating), Blood Pressure (Sphygmomanometer) and 
 *  Patient Position (Accelerometer)." 
 *
 *  In this example we read the values of the pulsioximeter sensor 
 *  and we show this values in the serial monitor
 *
 *  Copyright (C) 2012 Libelium Comunicaciones Distribuidas S.L.
 *  http://www.libelium.com
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  Version 2.0
 *  Author: Ahmad Saad & Luis Martin
 */

/*---------------------------------------------------------------------------------------------------
*  Be sure to measure the pulse and Oxygen saturation first, and then press button to upload data
----------------------------------------------------------------------------------------------------*/
#include <PinChangeInt.h>
#include <eHealth.h>

const byte MSG_CMD=0x00;        // Bluetooth transmit command, 0x00 message
const byte DATA_CMD=0x01;       // 0x01 Data
const byte FILE_SEND_CMD=0x02;  // 0x02 File Content

const int updateBtn = 4;     // button pin

int cont = 0;
int buttonState = 0;         // variable for reading the pushbutton status 

void setup() {
  Serial.begin(9600);        // Serial initialization, set baud rate, 9600 is the default baud rate of HC-06
  eHealth.initPulsioximeter(); //SPO2 initilization
  
  //Attach the inttruptions for using the pulsioximeter
  PCintPort::attachInterrupt(6, readPulsioximeter, RISING);
  
  pinMode(updateBtn,OUTPUT);  // button pin as OUTPUT
  delay(100);
  String s="Waiting...";      // Initial message on receiving device
  Serial.write(MSG_CMD);       // BT transmit command
  Serial.write(s.length());    // message length 
  Serial.println(s);           // transmit message to device 
}

void loop() {
  String outputData="";
  String s="Data OK!";
  char tmp[10];

  buttonState = digitalRead(updateBtn);    
  outputData+=eHealth.getBPM();               //get pulse reading
  outputData+=",";
    
  outputData+=eHealth.getOxygenSaturation();  //get Oxygen Saturation reading
  outputData+=",";
    
  float temperature = eHealth.getTemperature();//get temperature reading
  dtostrf(temperature,2,2,tmp); 
  outputData+=tmp;
  
  //  Serial.print("    Temperature (ºC): ");  
  if(buttonState)                               // when pressing button, upload data
  {
    Serial.write(DATA_CMD);                      // BT transmit command
    Serial.write(outputData.length());           // data length
    Serial.println(outputData);                  // transmit data to device
    
    Serial.write(MSG_CMD);                       // BT transmit command
    Serial.write(s.length());                    // message length 
    Serial.println(s);                           // transmit message to device  
    
  //  Serial.println("=========================================================="); 
    delay(500);    
  }
}

//Include always this code when using the pulsioximeter sensor
//=========================================================================
void readPulsioximeter(){  

  cont ++;

  if (cont == 50) { //Get only of one 50 measures to reduce the latency
    eHealth.readPulsioximeter();  
    cont = 0;
  }
}
