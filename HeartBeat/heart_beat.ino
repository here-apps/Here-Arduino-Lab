#include <SoftwareSerial.h>

SoftwareSerial BTSerial(10, 11);		// SoftwareSerial TX | RX for bluetooth

const int buttonPin = 2;
const int ledPin = 13;
const int sensorPin = A0;

int buttonState = 0;					// 儲存button是否按下的state

const double alpha = 0.75;				// smoothing參數 可自行調整0~1之間的值
const double beta = 1.0;				// find peak參數 可自行調整0~1之間的值
const int period = 20;					// sample脈搏的delay period

void setup()
{
	pinMode(buttonPin, INPUT);
	pinMode(ledPin, OUTPUT);
	Serial.begin(115200);				// Set the baud rate of the Serial Monitor
	BTSerial.begin(9600);				// HC-06 baud rate 預設為9600
}

void loop()
{
	buttonState = digitalRead(buttonPin);	// 讀取button的state

	// button press, start sensing heart rate.
	if (buttonState == HIGH) {
		digitalWrite(ledPin, 1);			// 亮起led表示開始測量心跳
		senseHeartRate();					// 測量心跳
        digitalWrite(ledPin, 0);			// 測量結束熄滅led
	}
}

void senseHeartRate()
{
	int count = 0;							// 記錄心跳次數
	double oldValue = 0;					// 記錄上一次sense到的值
	double oldChange = 0;					// 記錄上一次值的改變
        
	unsigned long startTime = millis();		// 記錄開始測量時間
	
	while(millis() - startTime < 10000) {	// sense 10 seconds
		int rawValue = analogRead(sensorPin);
		double value = alpha*oldValue + (1-alpha)*rawValue;		//smoothing value
	  
		//find peak
		double change = value-oldValue;				// 計算跟上一次值的改變量
		if (change>beta && oldChange<-beta) {		//heart beat
			count = count + 1;
			Serial.println(count);
		}
          
		oldValue = value;
		oldChange = change;
		delay(period);
	}
		
	BTSerial.println(count*6);			//use bluetooth to send result to android
}
