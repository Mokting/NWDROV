#include "Timer.h"

byte init_err = 1;
byte rxBuff[24];
byte dataReady = 0;
byte txData[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
unsigned long timeout = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial2.begin(19200);
  Serial2.setTimeout(50); //Set the timeout time as 50ms
}

void serialEvent2(){
  if(Serial2.available()){
    Serial2.readBytes(rxBuff, sizeof(rxBuff));
    dataReady = 1;
    for(int i=0; i<24; i++)
      Serial.print(rxBuff[i], DEC);
    Serial.print("\n");
  }
}

//Rubbish cmd consider time lost, need init again.

void cmdHandler(byte handler){
  if(handler == 1){
    txData[0] = 255;
    txData[1] = 170;
    txData[2] = 85;
    txData[3] = 0;
    txData[4] = 0;
    txData[5] = 0;
    txData[6] = 0;
    txData[7] = 85;
    txData[8] = 170;
    txData[9] = 255;
    for(char i=0; i<sizeof(txData); i++)
      Serial.print(txData[i], DEC);
    Serial.print("\n");
    Serial2.write(txData, sizeof(txData));
    Serial2.flush();

    String out = "Init successful\n";
    Serial.print(out);
    timeout = millis();
  }
  else if(handler == 2){
    String out = "Normal Operation\n";
    
    txData[0] = 255;
    txData[1] = 85;
    txData[8] = 170;
    txData[9] = 255;
    
    Serial2.write(txData, sizeof(txData));
    Serial2.flush();
    Serial.print(out);
    timeout = millis();
  }
  else if(handler == -1){
    //timeout = millis();
    //String out = "Error in decoding the command";
    //Serial.print(out);
  }

  for(byte i=0; i<10; i++)
    txData[i] = 0;
  dataReady = 0;
}

byte cmdReceive(byte *data){
  char handler = 0;
  byte head0, head1, tail;
  byte conFlag = 1;
  long tStart = millis();
  byte cmd[11];
  
  #define waitTime (50)

  Serial.println("CMD Receive");
  while(conFlag == 1 && (millis() - tStart < waitTime)){
    for(byte i=0; i<24; i++){
      //Serial.print("Data");
      //Serial.print(i, DEC);
      //Serial.print(": ");
      //Serial.println(data[i], DEC);
      if(data[i] == 255 || data[i] == 140){
        //Serial.println("Eq!");
        for(byte j=0; j<11; j++)
          cmd[j] = data[i+j+1];
        if(cmd[10] == 255 || cmd[10] == 150){
          //Serial.println("CMD CHECKING");
          conFlag = 0;
          break;
        }
      }
    }
  }
  if(conFlag == 1){
    Serial.println("conFlag1");
    handler = -1;
  }
  else{
    if(cmd[10] == 255)
      handler = 1;  //Init
    else if(cmd[10] == 150)
      handler = 2;  //Normal Command
    else{ 
      handler = -1;  //Error
      Serial.println("Last header Failed");
    }
    if(handler == 1){
      if(cmd[0] != 170 || cmd[1] != 0 || cmd[2] != 85 || cmd[3] != 170 || cmd[4] != 0 || cmd[5] != 85 || cmd[6] != 170 || cmd[7] != 0 || cmd[8] != 85 || cmd[9] != 0)
        handler = -1;
        Serial.println("init failed");  
    }
  }
  return handler;
}

void loop() {
  // put your main code here, to run repeatedly
  byte handler = 0;

  //Only enter this command handle loop when serial port has data
  if(dataReady == 1){
    handler = cmdReceive(rxBuff);
    cmdHandler(handler);

    //clean the buffer
    for(byte i=0; i<24; i++)
      rxBuff[i] = 0;
  }

  //timeout value will be updated for every successful/valid data
  //It stop update for each failed checking
  if(millis() - timeout > 1000){
    //Stop machine working and wait for initaiation from controller
    String out = "Timeout!\n";
    Serial.print(out);
  }
}

