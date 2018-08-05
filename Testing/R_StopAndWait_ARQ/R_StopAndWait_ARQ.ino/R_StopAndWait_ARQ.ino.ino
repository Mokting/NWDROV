#include "Timer.h"

byte init_err = 1;
byte rxData[12] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
byte dataReady = 0;
byte txData[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
long timeout = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial2.begin(19200);
  Serial2.setTimeout(50);
}

//once hav err go here loop
/*
byte rs485_init(void){
  #define waitTime (50)
  
  byte tStart = millis();
  while(!Serial2.available() && (millis() - tStart < waitTime));

  if(Serial2.available()){
    byte data[5];
    Serial2.readBytes(data, sizeof(data));
    
    if(data[0] != 255 || data[1] != 170 || data[2] != 0 || data[3] != 85 || data[4] != 255){
      byte err_seq[5] = {255, 85, 0, 170, 255};
      Serial2.write(err_seq, sizeof(err_seq));
      Serial2.flush();
      Serial.println("Problem!");
    }
    else{
      byte init_seq[5] = {255, 170, 0, 85, 255};
      Serial2.write(init_seq, sizeof(init_seq));
      Serial.write("OK!");
      return 0;
    }
  }
  else
    return 1; //error in init
}
*/

void serialEvent2(){
  if(Serial2.available()){
    Serial2.readBytes(rxData, sizeof(rxData));
    dataReady = 1;
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
    String out = "Error in decoding the command";
    Serial.print(out);
  }

  for(byte i=0; i<10; i++)
    txData[i] = 0;
  dataReady = 0;
}

byte cmdReceive(byte *data){
  char handler = 0;
  if(data[0] == 255 && data[11] == 255)
    handler = 1;  //Init
  else if(data[0] == 140 && data[11] == 150)
    handler = 2;  //Normal Command
  else 
    handler = -1;  //Error

  if(handler == 1){
    if(data[1] != 170 || data[2] != 0 || data[3] != 85 || data[4] != 170 || data[5] != 0 || data[6] != 85 || data[7] != 170 || data[8] != 0 || data[9] != 85 || data[10] != 0)
      handler = -1;  
  }
  return handler;
}

void loop() {
  // put your main code here, to run repeatedly
  byte handler = 0;
  if(dataReady == 1){
    handler = cmdReceive(rxData);
    cmdHandler(handler);

    for(byte i=0; i<12; i++)
      rxData[i] = 0;
  }

  if(millis() - timeout > 1000){
    String out = "Timeout!\n";
    Serial.print(out);
  }
}

