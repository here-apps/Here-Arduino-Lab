#include <SoftwareSerial.h>
#include <Wire.h>
#include <SeeedGrayOLED.h>
#include <SPI.h>
#include <MFRC522.h>

/*
 * MFRC522 - Library to use ARDUINO RFID MODULE KIT 13.56 MHZ WITH TAGS SPI W AND R BY COOQROBOT.
 * The library file MFRC522.h has a wealth of useful info. Please read it.
 * The functions are documented in MFRC522.cpp.
 *
 * Based on code Dr.Leong   ( WWW.B2CQSHOP.COM )
 * Created by Miguel Balboa (circuitito.com), Jan, 2012.
 * Rewritten by Søren Thing Andersen (access.thing.dk), fall of 2013 (Translation to English, refactored, comments, anti collision, cascade levels.)
 * Released into the public domain.
 *
 * This sample shows how to setup a block on a MIFARE Classic PICC to be in "Value Block" mode.
 * In Value Block mode the operations Increment/Decrement/Restore and Transfer can be used.
 * 
 ----------------------------------------------------------------------------- empty_skull 
 * Aggiunti pin per arduino Mega
 * add pin configuration for arduino mega
 * http://mac86project.altervista.org/
 ----------------------------------------------------------------------------- Nicola Coppola
 * Pin layout should be as follows:
 * Signal     Pin              Pin               Pin
 *            Arduino Uno      Arduino Mega      MFRC522 board
 * ------------------------------------------------------------
 * Reset      9                5                 RST
 * SPI SS     10               53                SDA
 * SPI MOSI   11               51                MOSI
 * SPI MISO   12               50                MISO
 * SPI SCK    13               52                SCK
 *
 * The reader can be found on eBay for around 5 dollars. Search for "mf-rc522" on ebay.com. 
 */

#define SS_PIN 10
#define RST_PIN 9
MFRC522 mfrc522(SS_PIN, RST_PIN);	// Create MFRC522 instance.
SoftwareSerial BTSerial(2, 3); // RX | TX
int checkState;

void setup() {
	Serial.begin(57600);	// Initialize serial communications with the PC
	SPI.begin();		// Init SPI bus
        BTSerial.begin(9600);   // Init Bluetooth SoftwareSerial
	mfrc522.PCD_Init();	// Init MFRC522 card
  	Serial.println("Scan a MIFARE Classic PICC to check.");
  
        Wire.begin();          
        SeeedGrayOled.init();  //initialize SEEED Gray OLED display        
        SeeedGrayOled.clearDisplay();           //Clear Display.  
        SeeedGrayOled.setVerticalMode();        // Set to vertical mode for displaying text
        SeeedGrayOled.setTextXY(3,0);          
        SeeedGrayOled.putString(" Card here!"); //Print string
}

void loop() {
        //variable declaration        
        String outputData="";           //string sent by BT
        String uidstr="";               //card uid string
  
        /*---------------------mfrc522 -------------------------------*/        
	// Look for new cards
	if ( ! mfrc522.PICC_IsNewCardPresent()) {
		return;
	}

	// Select one of the cards
	if ( ! mfrc522.PICC_ReadCardSerial()) {
		return;
	}
	// Now a card is selected. The UID and SAK is in mfrc522.uid.
	
	// Dump UID
	Serial.print("Card UID:");
	for (byte i = 0; i < mfrc522.uid.size; i++) {
                if(mfrc522.uid.uidByte[i] < 0x10)
                {
                  uidstr+=" 0";              
                  Serial.print(" 0");  
                }
                else
                {
                  uidstr+=" ";
                  Serial.print(" ");                  
                }
                uidstr+= String(mfrc522.uid.uidByte[i],HEX);
		Serial.print(mfrc522.uid.uidByte[i], HEX);
	} 
	Serial.println();

	// Dump PICC type
	byte piccType = mfrc522.PICC_GetType(mfrc522.uid.sak);
	if (	piccType != MFRC522::PICC_TYPE_MIFARE_MINI 
		&&	piccType != MFRC522::PICC_TYPE_MIFARE_1K
		&&	piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
		Serial.println("This sample only works with MIFARE Classic cards.");
		return;
	}

	// Prepare key - all keys are set to FFFFFFFFFFFFh at chip delivery from the factory.
	MFRC522::MIFARE_Key key;
	for (byte i = 0; i < 6; i++) {
		key.keyByte[i] = 0xFF;
	}

       // In this sample we use the second sector (ie block 4-7). the first sector is = 0
        // scegliere settore di lettura da 0 = primo settore 
        byte sector         = 0;
        // block sector 0-3(sector0) 4-7(sector1) 8-11(sector2)
        // blocchi di scrittura da 0-3(sector0) 4-7(sector1) 8-11(sector2)
        byte valueBlockA    = 0;        //manufactor information block
        byte valueBlockB    = 1;
        byte valueBlockC    = 2;
        byte trailerBlock   = 3;
        
        byte value1BlockIn[] = { 0,0,0,1,  0,0,0,0, 0,0,0,0,  0,0,0,0,   valueBlockB,~valueBlockB,valueBlockB,~valueBlockB };        //value1Block value for "check in" state
        byte value1BlockOut[] = { 0,0,0,0,  0,0,0,0, 0,0,0,0,  0,0,0,0,   valueBlockB,~valueBlockB,valueBlockB,~valueBlockB };       //value1Block value for "check out" state
        
        byte status;
        // Authenticate using key A.
        // avvio l'autentificazione A
        //Serial.println("Authenticating using key A...");
        status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, &key, &(mfrc522.uid));
        if (status != MFRC522::STATUS_OK) {
                Serial.print("PCD_Authenticate() failed: ");
                Serial.println(mfrc522.GetStatusCodeName(status));
                return;
        }
        // Authenticate using key B.
        // avvio l'autentificazione B
        //Serial.println("Authenticating again using key B...");
        status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_B, trailerBlock, &key, &(mfrc522.uid));
        if (status != MFRC522::STATUS_OK) {
                Serial.print("PCD_Authenticate() failed: ");
                Serial.println(mfrc522.GetStatusCodeName(status));
                return;
        }
        
        byte buffer[18];
        byte size = sizeof(buffer);
        // change this: valueBlockC , for read anather block
        // cambiate valueBlockA per leggere un altro blocco
        status = mfrc522.MIFARE_Read(valueBlockB, buffer, &size);               
        
        /*------------check for the fourth bit of valueblockB, 1: already checked in, 0: already checked out---------- */
        if(buffer[3]==0)                              // No check
        {
                  status = mfrc522.MIFARE_Write(valueBlockB, value1BlockIn, 16);
                  if (status != MFRC522::STATUS_OK) {
                          Serial.print("MIFARE_Write() failed: ");
                          Serial.println(mfrc522.GetStatusCodeName(status));
                  }          
          Serial.println("Check in");
          outputData=outputData+uidstr+",1";
          /*----------Send check in information through Bluetooth...............*/          
          BTSerial.println(outputData);
          Serial.println(outputData);
          /*----------Print check in information on OLED screen-----------------*/
          SeeedGrayOled.setTextXY(6,0);
          SeeedGrayOled.putString(" Hello!!");      //Print string          
          SeeedGrayOled.setTextXY(8,0);
          SeeedGrayOled.putString(" Check in!");      //Print string
          Serial.println(buffer[3]);      
          checkState =1;
        }else if(buffer[3]==1)                   // Already check in
        {
          status = mfrc522.MIFARE_Write(valueBlockB, value1BlockOut, 16);
          if (status != MFRC522::STATUS_OK) {
            Serial.print("MIFARE_Write() failed: ");
            Serial.println(mfrc522.GetStatusCodeName(status));
          }         
          Serial.println("Check out");
          outputData=outputData+uidstr+",0";
          /*----------Send check out information through Bluetooth...............*/                    
          BTSerial.println(outputData);
          /*----------Print check in information on OLED screen-----------------*/          
          Serial.println(outputData);
          SeeedGrayOled.setTextXY(6,0);
          SeeedGrayOled.putString(" Goodbye!!");      //Print string
          SeeedGrayOled.setTextXY(8,0);
          SeeedGrayOled.putString(" Check out");      //Print string
          Serial.println(buffer[3]);
          checkState =0;
        }else  //error checking, output error msg
        {
          SeeedGrayOled.setTextXY(6,0);
          SeeedGrayOled.putString(" Error!!");      //Print number
          SeeedGrayOled.setTextXY(8,0);
          SeeedGrayOled.putString(" Reset");      //Print number         
          Serial.println(buffer[3]);          
                  status = mfrc522.MIFARE_Write(valueBlockB, value1BlockIn, 16);
                  if (status != MFRC522::STATUS_OK) {
                          Serial.print("MIFARE_Write() failed: ");
                          Serial.println(mfrc522.GetStatusCodeName(status));
                  }    
           checkState =999;                
        }     
        delay(2000);
        
        // Halt PICC
        mfrc522.PICC_HaltA();

        // Stop encryption on PCD
        mfrc522.PCD_StopCrypto1();
}
