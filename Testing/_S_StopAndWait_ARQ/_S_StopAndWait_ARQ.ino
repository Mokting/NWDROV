#include "Timer.h"

byte init_err = 1;
byte rxFlag = 1;
byte rxErr = 0;
long timeout = 0;
Timer tim1;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial2.begin(19200);
  Serial2.setTimeout(50);
  
  init_err = rs485_init();
  if(init_err == 0)
    Serial.println("RS485 Init successfully");
  tim1.every(100, cmdSend);
}

byte rs485_init(void){
  byte init_seq[12] = {255, 170, 0, 85, 170, 0, 85, 170, 0, 85, 0, 255};
  byte err = 0;
  #define waitTime (50) //in ms

  Serial2.write(init_seq, sizeof(init_seq));
  Serial2.flush();  
  unsigned long tStart = millis();
  while(!Serial2.available() && (millis() - tStart < waitTime));
  
  if(Serial2.available()){
    byte head0, head1;
    byte conFlag = 1;  
    tStart = millis();
    byte init = 0;
    
    while(conFlag == 1 && millis() - tStart < waitTime){
      if(init == 0){
        Serial2.readBytes(&head0, 1);
        Serial2.readBytes(&head1, 1);
        init = 1;
      }
      else{
        head0 = head1;
        Serial2.readBytes(&head1, 1);
      }
      if(head0 == 255 && head1 == 170)
        conFlag = 0;
    }

    if(conFlag == 1){
      Serial.println("Useless");
      err = 1;
    }
    else{
      byte data[8];
      Serial2.readBytes(data, sizeof(data));
      Serial.println("Received!");
      
      if(data[0] != 85 || data[1] != 0 || data[2] != 0 || data[3] != 0 || data[4] != 0 || data[5] != 85 || data[6] != 170 || data[7] != 255){
        err = 1;
        Serial.println("Err in decoding");
        for(char i=0; i<8; i++)
          Serial.print(data[i], DEC);
        Serial.print("\n");
      }
      else
        err = 0;
    }
  }
  else
    err = 1;
  
  return err;
}

void cmdSend(void){
  if(init_err != 0){
    init_err = rs485_init();
    Serial.println("Resend");

    if(init_err == 0)
      rxErr = 0;
  }
  else{
    //normal cmd Sending Routine
    byte sendCmd[12] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    sendCmd[0] = 140;
    sendCmd[11] = 150;
    Serial2.write(sendCmd, sizeof(sendCmd));
    Serial.print("CMD Sent\n");
    rxFlag = dataReceive();
    if(rxFlag != 0){
      rxErr++;
      
    }
  }
  Serial.println(rxErr, DEC);
  //Serial.print("i: ");
  //Serial.println(init_err, DEC);
}

byte dataReceive(void){
  #define waitTime1 (50) //in ms
  byte err = 0;
  
  unsigned long tStart = millis();
  while(!Serial2.available() && (millis() - tStart < waitTime));

  if(Serial2.available()){
    byte head0, head1;
    byte conFlag = 1;
    byte init = 0;
    tStart = millis();
    Serial.println("Enter DR");
    
   while(conFlag == 1 && millis() - tStart < waitTime1){
      Serial.println("DLLM");
      if(init == 0){
        Serial2.readBytes(&head0, 1);
        Serial2.readBytes(&head1, 1);
        init = 1;
      }
      else{
        head0 = head1;
        Serial2.readBytes(&head1, 1);
      }
      if(head0 == 255 && head1 == 85)
        conFlag = 0;

      Serial.print("rh0: ");
      Serial.println(head0, DEC);
      Serial.print("rh1: ");
      Serial.println(head1, DEC);
    }

    if(conFlag == 1){
      err = 1;
      Serial.println("ConFlag!");
    }
    else{
      byte data[8];
      Serial2.readBytes(data, sizeof(data));
      
      if(data[6] != 170 || data[7] != 255){
        Serial.println("Data Corr");
        err = 1;
      }
      else{
        String out = "ACK Received!\n";
        Serial.print(out);
        err = 0;
      }
    }
  }
  else{
    Serial.println("No ACK Received");
    err = 1;
  }
  return err;
}

void loop() {
  tim1.update();
  if(rxErr >= 5){
    init_err = 1;
    rxErr = 5;
    String out = "ACK ERR, TERMINATE\n";
    Serial.print(out);
  }
}
