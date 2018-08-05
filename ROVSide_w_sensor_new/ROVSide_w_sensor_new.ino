#include "Timer.h"
#include <Servo.h>
#include <OneWire.h>

#define fLpin (10)
#define bLpin (11)
#define holdPWM (1450)


#define gMotorPin    (22)
#define gDirPin      (23)
#define clrMotorPin  (24)
#define clrDirPin    (25)
#define gripPin      (12)
#define tempPin      (31)
#define pumpPin      (44)
#define pressPin     (A0)

//#define debug         

Servo fLight, bLight, gripper, pump;
Servo motor[8];
Timer tim1, manPwm, gripPwm;
OneWire ds(tempPin);

const char motorPin[8] = {2, 3, 4, 5, 6, 7, 8, 9};
byte handler = 0;
short on9 = 0;
short dllm = 0;

short pwm[8] = {1450, 1450, 1450, 1450, 1450, 1450, 1450, 1450};
short lastPwm[8] = {1450, 1450, 1450, 1450, 1450, 1450, 1450, 1450};
byte sendStatus1 = 0;
byte sendStatus2 = 0;
byte diff = 0;
byte lastBuf[2];
byte addr[8]; //for temp sensor
byte clrSpeed = 0;
byte clrEnable = 0;
byte pwmCount = 0;
byte gripCount = 0;
byte gmEnable = 0;
byte upCount = 0;

void allStop(void){
  upCount++;

  if(upCount >= 20)
    upCount = 20;
    
  gripper.writeMicroseconds(1500);

  /*
  if(upCount < 20)
    digitalWrite(gMotorPin, LOW);
  else
    digitalWrite(gMotorPin, HIGH);
  */
  
  digitalWrite(gDirPin, HIGH);
  digitalWrite(clrMotorPin, HIGH);
   
  pump.writeMicroseconds(1450);
  fLight.write(1900);
  bLight.write(1900);
  for(char i=0; i<8; i++)
    motor[i].writeMicroseconds(1450);
}

void setup() {
  //Init the pins
  fLight.attach(fLpin);
  bLight.attach(bLpin);
  gripper.attach(gripPin);
  pump.attach(pumpPin);

  pinMode(gMotorPin, OUTPUT); 
  pinMode(gDirPin, OUTPUT); 
  pinMode(clrMotorPin, OUTPUT); 
  pinMode(clrDirPin, OUTPUT); 
  
  for(char i=0; i<8; i++){
    motor[i].attach(motorPin[i]);
    motor[i].writeMicroseconds(holdPWM);
  }
  fLight.write(1500);
  bLight.write(1500);
  pump.writeMicroseconds(holdPWM);
  digitalWrite(clrMotorPin, HIGH);
  digitalWrite(gMotorPin, HIGH);
  gripper.writeMicroseconds(1500);
  Serial.begin(9600);
  Serial3.begin(28800);
  
  //oneWire init for using temp sensor
  if ( !ds.search(addr)) {
      //no more sensors on chain, reset search
      ds.reset_search();
      //return -1000;
  }
  
  if ( OneWire::crc8( addr, 7) != addr[7]) {
      Serial.println("CRC is not valid!");
      //return -1000;
  }

  if ( addr[0] != 0x10 && addr[0] != 0x28) {
      Serial.print("Device is not recognized");
      //return -1000;
  }
  delay(10);
  
  tim1.every(50, cmdReceive);
  manPwm.every(2, cleanSpeed);
  gripPwm.every(1, gripCtrl);
}

void loop() {
  tim1.update();
  manPwm.update();
  gripPwm.update();
}

void cmdExecute(void){
  //cmd decode
  byte bLstatus = sendStatus1 & (B00000011);
  byte fLstatus = (sendStatus1 >> 2) & (B00000011);
  byte grip = (sendStatus1 >> 4) & (B00000011);
  clrSpeed = (sendStatus2 >> 3) & (B00000011);
  byte gM = sendStatus2 & (B00000011);
  byte clrMotor = (sendStatus2 >> 2) & (B00000001);

  
  if(gM == 0){
    //digitalWrite(gMotorPin, LOW);
    gmEnable = 1;
    upCount = 0;
    digitalWrite(gDirPin, HIGH);
  }
  else if(gM == 2){
    //digitalWrite(gMotorPin, LOW);
    gmEnable = 1;
    upCount = 0;
    digitalWrite(gDirPin, LOW);
  }
  else{
    //digitalWrite(gMotorPin, HIGH);
    gmEnable = 0;
  }
  
  if(grip == 0)
    gripper.writeMicroseconds(1700);
  else if(grip == 2)
    gripper.writeMicroseconds(1100);
  else
    gripper.writeMicroseconds(1500);

  if(clrMotor == 1){
      clrEnable = 1;
     //digitalWrite(clrMotorPin, LOW);
     pump.writeMicroseconds(1000);
    }
    else{
      clrEnable = 0;
      //digitalWrite(clrMotorPin, HIGH);
      pump.writeMicroseconds(1450);
    }
  
  switch(fLstatus){
    case 0:
      fLight.writeMicroseconds(1100);
      break;

    case 1:
      fLight.writeMicroseconds(1300);
      break;
      

    case 2:
      fLight.writeMicroseconds(1600);
      break;

    case 3:
      fLight.writeMicroseconds(1900);
      break;
  }

  switch(bLstatus){
    case 0:
      bLight.writeMicroseconds(1100);
      break;

    case 1:
      bLight.writeMicroseconds(1500);
      break;

    case 2:
      bLight.writeMicroseconds(1700);
      break;

    case 3:
      bLight.writeMicroseconds(1900);
      break;
  }
  for(char i=0; i<8; i++){
    if(pwm[i] < 1000)
      pwm[i] = 1000;
    else if(pwm[i] > 1900)
      pwm[i] = 1900;
      
    motor[i].writeMicroseconds(pwm[i]);
  }
  
  #ifdef debug
  Serial.print("fLstatus: ");
  Serial.println(fLstatus);
  Serial.print("bLstatus: ");
  Serial.println(bLstatus);
  Serial.print("grip: ");
  Serial.println(grip);
  Serial.print("gM: ");
  Serial.println(gM);
  
  Serial.print("clrSpeed: ");
  Serial.println(clrSpeed);
  Serial.print("clrMotor: ");
  Serial.println(clrMotor);
  Serial.print("Motor: ");
  for(char i=0; i<8; i++){
    Serial.print(pwm[i]);
    Serial.print(", ");
    if(i==7)
      Serial.println("\n");
  
  }
  #endif
}

//Very on0 command handling function. Will integrate the newly written communcation code once i come back
void cmdReceive(void){
  if(Serial3.available()){
    byte buff[12]; //cmd from onshore board has 22 byte, check one by one
    Serial3.readBytes(buff, sizeof(buff)); //correct)
    
    if(buff[0] == 140 && buff[1] == 255 && buff[2] == 255 && buff[11] == 150){
      handler = 1;
      sendData();
    }
    else if(buff[0] == 140 && buff[11] == 150){
      sendStatus1 = buff[1];
      sendStatus2 = buff[2];
      pwm[0] = map(buff[3], 0, 255, 1000, 1900);
      pwm[1] = map(buff[4], 0, 255, 1000, 1900);
      pwm[2] = map(buff[5], 0, 255, 1000, 1900);
      pwm[3] = map(buff[6], 0, 255, 1000, 1900);
      pwm[4] = map(buff[7], 0, 255, 1000, 1900);
      pwm[5] = map(buff[8], 0, 255, 1000, 1900);
      pwm[6] = map(buff[9], 0, 255, 1000, 1900);
      pwm[7] = map(buff[10], 0, 255, 1000, 1900);
      for(char i=0; i<2; i++)
        lastBuf[i] = buff[i+1];
      for(char i=0; i<8; i++)
        lastPwm[i] = pwm[i];
      handler = 0;
    }
    else
      handler = 2;
      
    if(handler == 0){
      on9 = 0;
      cmdExecute();
    }
    else if(handler == 2){
      #ifdef debug
      Serial.println("\nError in decoding the command");
      //Serial.print("diff: ");
      //Serial.print(diff, DEC);
      //Serial.print("\n");
      Serial.print("\n");  
      #endif
      
      sendStatus1 = lastBuf[0];
      sendStatus2 = lastBuf[1];
      for(char i=0; i<8; i++)
        pwm[i] = lastPwm[i];  
      on9++;
      
      byte discard;
      for(byte i=0; i<12; i++)
        Serial3.readBytes(&discard, 1);
    }

    //error handling
    if(on9 >= 40){
      on9 = 40;
    }
    if(on9 == 40)
      allStop();
  } 
  else {
    #ifdef debug
    Serial.println("No Command Received!!!");
    #endif
    on9++;
    
    if(on9 >= 40)
      on9 = 40;
    if(on9 == 40)
      allStop();
  }
}

void sendData(void){
  Serial3.print(analogRead(pressPin), DEC);
  Serial3.print("@");
  Serial3.print(getTemp());
  Serial3.print("!");
  Serial3.flush();
}

float getTemp(){
  //returns the temperature from one DS18S20 in DEG Celsius

  byte data[12];
  
  ds.reset();
  ds.select(addr);
  ds.write(0x44,1); // start conversion, with parasite power on at the end

  byte present = ds.reset();
  ds.select(addr);    
  ds.write(0xBE); // Read Scratchpad

  
  for (int i = 0; i < 9; i++) { // we need 9 bytes
    data[i] = ds.read();
  }
  
  ds.reset_search();
  
  byte MSB = data[1];
  byte LSB = data[0];

  float tempRead = ((MSB << 8) | LSB); //using two's compliment
  float TemperatureSum = tempRead / 16;
  
  return TemperatureSum;
}

void cleanSpeed(void){
  // 45HZ manual generated PWM using digital Write
  pwmCount++;
  if(pwmCount >= 10)
    pwmCount = 0;
  
  if(clrEnable == 1){
    if(clrSpeed == 0){
       if(pwmCount < 2)
          digitalWrite(clrMotorPin, LOW);
        else
          digitalWrite(clrMotorPin, HIGH);
    }
    else if(clrSpeed == 1){
      if(pwmCount < 6)
        digitalWrite(clrMotorPin, LOW);
      else
        digitalWrite(clrMotorPin, HIGH);
    }
    else
      digitalWrite(clrMotorPin, LOW);
 }
  else
    digitalWrite(clrMotorPin, HIGH);
}

void gripCtrl(void){
  gripCount++;
  if(gripCount >1)
    gripCount = 0;

  if(gmEnable == 1){
    if(gripCount == 0)
      digitalWrite(gMotorPin, LOW);
    else
      digitalWrite(gMotorPin, HIGH);
  } 
  else
    digitalWrite(gMotorPin, HIGH);
}

