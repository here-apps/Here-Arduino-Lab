#include <SPI.h>
#include <SD.h>

#include <arduino.h>
#include <SoftwareSerial.h>

#define PIC_PKT_LEN    128        //data length of each read, dont set this too big because ram is limited
#define PIC_FMT_VGA    7
#define PIC_FMT_CIF    5
#define PIC_FMT_OCIF   3
#define CAM_ADDR       0
#define CAM_SERIAL     softSerial

#define PIC_FMT        PIC_FMT_VGA

File myFile;
SoftwareSerial softSerial(2, 3);   //rx,tx (11-13 is used by sd shield)
SoftwareSerial BTSerial(6, 7);     // RX | TX for BT

int IRswitch = 8;                  //IR motion pin
int led = 10;                      //LED pin
boolean val = false;               //motion val, 0: no    1: yes 

const byte cameraAddr = (CAM_ADDR << 5);  // addr
unsigned long picTotalLen = 0;            // picture length
int picNameNum = 0;
int n = 0;  

/*********************************************************************/
void setup()
{
  Serial.begin(115200);

  Serial.println("Initializing SD card....");
  pinMode(4,OUTPUT);                        // CS pin of SD Card module
  
  if (!SD.begin(4)) {
    Serial.print("sd init failed");
    return;
  }
  Serial.println("sd init done.");
  
  CAM_SERIAL.begin(9600);       //cant be faster than 9600, maybe difference with diff board.
  pinMode(IRswitch,INPUT);      //initialize the IRswitch pin as INPUT     
  pinMode(led,OUTPUT);          //LED pin as OUTPUT
  initialize();
}
/*********************************************************************/
void loop()
{ 
  int alert=1;
  val = digitalRead(IRswitch);   //read IR motion pin； 
  Serial.println("Monitoring...");
  if(val)                        //if high, intruder detects!!!Alert!!!
  {
    Serial.println("Intruder!!!!");   
    BTSerial.println(alert);     //Send alert info. through BT
    digitalWrite(led,HIGH);      //Led light on
    //taking picture
    delay(200);
    if (n == 0) preCapture();
    Capture();                    //taking photo and saving in the buffer
    Serial.print("Saving picture...");
    GetData();                    //save picture into SD card

    Serial.print("\r\nDone");  
    Serial.println(n);  
    n++;  
  }
  else
    digitalWrite(led,LOW);         //no motion detect, led light off 
  delay(3000);                     //delay for stable 
}
/********************function of Serial camera kit*****************************************/
void clearRxBuf()
{
  while (CAM_SERIAL.available()) 
  {
    CAM_SERIAL.read(); 
  }
}
/*********************************************************************/
void sendCmd(char cmd[], int cmd_len)
{
  for (char i = 0; i < cmd_len; i++) CAM_SERIAL.write(cmd[i]); 
}
/*********************************************************************/
int readBytes(char *dest, int len, unsigned int timeout)
{
  int read_len = 0;
  unsigned long t = millis();
  while (read_len < len)
  {
    while (CAM_SERIAL.available()<1)
    {
      if ((millis() - t) > timeout)
      {
        return read_len;
      }
    }
    *(dest+read_len) = CAM_SERIAL.read();
//    Serial.write(*(dest+read_len));
    read_len++;
  }
  return read_len;
}
/*********************************************************************/
void initialize()
{   
  char cmd[] = {0xaa,0x0d|cameraAddr,0x00,0x00,0x00,0x00} ;  
  unsigned char resp[6];

  Serial.print("initializing camera...");
  
  while (1) 
  {
    sendCmd(cmd,6);
    if (readBytes((char *)resp, 6,1000) != 6)
    {
      Serial.print(".");
      continue;
    }
    if (resp[0] == 0xaa && resp[1] == (0x0e | cameraAddr) && resp[2] == 0x0d && resp[4] == 0 && resp[5] == 0) 
    {
      if (readBytes((char *)resp, 6, 500) != 6) continue; 
      if (resp[0] == 0xaa && resp[1] == (0x0d | cameraAddr) && resp[2] == 0 && resp[3] == 0 && resp[4] == 0 && resp[5] == 0) break; 
    }
  }  
  cmd[1] = 0x0e | cameraAddr;
  cmd[2] = 0x0d;
  sendCmd(cmd, 6); 
  Serial.println("\nCamera initialization done.");
}
/*********************************************************************/
void preCapture()
{
  char cmd[] = { 0xaa, 0x01 | cameraAddr, 0x00, 0x07, 0x00, PIC_FMT };  
  unsigned char resp[6]; 
  
  while (1)
  {
    clearRxBuf();
    sendCmd(cmd, 6);
    if (readBytes((char *)resp, 6, 100) != 6) continue; 
    if (resp[0] == 0xaa && resp[1] == (0x0e | cameraAddr) && resp[2] == 0x01 && resp[4] == 0 && resp[5] == 0) break; 
  }
}
void Capture()
{
  char cmd[] = { 0xaa, 0x06 | cameraAddr, 0x08, PIC_PKT_LEN & 0xff, (PIC_PKT_LEN>>8) & 0xff ,0}; 
  unsigned char resp[6];

  while (1)
  {
    clearRxBuf();
    sendCmd(cmd, 6);
    if (readBytes((char *)resp, 6, 100) != 6) continue;
    if (resp[0] == 0xaa && resp[1] == (0x0e | cameraAddr) && resp[2] == 0x06 && resp[4] == 0 && resp[5] == 0) break; 
  }
  cmd[1] = 0x05 | cameraAddr;
  cmd[2] = 0;
  cmd[3] = 0;
  cmd[4] = 0;
  cmd[5] = 0; 
  while (1)
  {
    clearRxBuf();
    sendCmd(cmd, 6);
    if (readBytes((char *)resp, 6, 100) != 6) continue;
    if (resp[0] == 0xaa && resp[1] == (0x0e | cameraAddr) && resp[2] == 0x05 && resp[4] == 0 && resp[5] == 0) break;
  }
  cmd[1] = 0x04 | cameraAddr;
  cmd[2] = 0x1;
  while (1) 
  {
    clearRxBuf();
    sendCmd(cmd, 6);
    if (readBytes((char *)resp, 6, 100) != 6) continue;
    if (resp[0] == 0xaa && resp[1] == (0x0e | cameraAddr) && resp[2] == 0x04 && resp[4] == 0 && resp[5] == 0)
    {
      if (readBytes((char *)resp, 6, 1000) != 6)
      {
        continue;
      }
      if (resp[0] == 0xaa && resp[1] == (0x0a | cameraAddr) && resp[2] == 0x01)
      {
        picTotalLen = (resp[3]) | (resp[4] << 8) | (resp[5] << 16); 
        Serial.print("picTotalLen:");
        Serial.println(picTotalLen);
        break;
      }
    }
  }  
}
/*********************************************************************/
void GetData()
{
  unsigned int pktCnt = (picTotalLen) / (PIC_PKT_LEN - 6); 
  if ((picTotalLen % (PIC_PKT_LEN-6)) != 0) pktCnt += 1;
  
  char cmd[] = { 0xaa, 0x0e | cameraAddr, 0x00, 0x00, 0x00, 0x00 };  
  unsigned char pkt[PIC_PKT_LEN];
  
  char picName[] = "pic00.jpg";
  picName[3] = picNameNum/10 + '0';
  picName[4] = picNameNum%10 + '0';
  
  if (SD.exists(picName))
  {
    SD.remove(picName);
  }
  
  myFile = SD.open(picName, FILE_WRITE); 
  if(!myFile){
    Serial.println("myFile open fail...");
  }
  else{
    for (unsigned int i = 0; i < pktCnt; i++)
    {
      cmd[4] = i & 0xff;
      cmd[5] = (i >> 8) & 0xff;
      
      int retry_cnt = 0;
    retry:
      delay(10);
      clearRxBuf(); 
      sendCmd(cmd, 6); 
      uint16_t cnt = readBytes((char *)pkt, PIC_PKT_LEN, 200);
      
      unsigned char sum = 0; 
      for (int y = 0; y < cnt - 2; y++)
      {
        sum += pkt[y];
      }
      if (sum != pkt[cnt-2])
      {
        if (++retry_cnt < 100) goto retry;
        else break;
      }
      
      myFile.write((const uint8_t *)&pkt[4], cnt-6);         //write to file in SD card, subtracting 4 bytes header and 2 bytes tailor
      //if (cnt != PIC_PKT_LEN) break;
    }
    cmd[4] = 0xf0;
    cmd[5] = 0xf0; 
    sendCmd(cmd, 6); 
  }
  myFile.close();
  Serial.println("Data transferring ok");
  picNameNum ++;
}
