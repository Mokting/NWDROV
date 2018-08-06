#include <PS2X_lib.h>
#include <Servo.h>
#include "Timer.h"

#define PS2_DAT     50
#define PS2_CMD     51
#define PS2_CLK     52
#define PS2_SEL     53
#define pressures   false
#define rumble      false

#define fLpin (10)
#define bLpin (11)
#define startPWM (1450)
#define pi (3.14159)

#define gMotorPin    (22)
#define gDirPin      (23)
#define clrMotorPin  (24)
#define clrDirPin    (25)
#define gripPin      (12)
#define pumpPin      (44)

//#define debug         0
#define decel         0


PS2X ps2x;
Timer tim1, tim2;
Servo fLight, bLight, gripper, pump;
Servo motor[8];
char ps2Err;
bool locked = true;
const char motorPin[8] = {2,3,4,5,6,7,8,9};

//char axisSpeed  = 1; //3 state
char rotL = 0;              //z axis rotate left: 0 no, 1 yes
char rotR = 0;              //z axis rotate right: 0 no, 1 yes
char lightSel   = 0;        //0 front, 1 black
char light[2]   = {0,0};    //4 state: Off, brightness 1, 2, 3
bool clrMotor   = 0;        //clearing motor: On, Off
char pumpCtrl = 0;          //pump control: 0 Off, 1 On. Useless now as the pump will on together with clearning motor
bool invCtrl = 0;           //0 Gripper as front, 1 clearing motor as front
char gM = 1;                //gripper motor: 0 down, 1 off, 2 up
char grip = 1;              //grippe: 0 open. 1 stop, 2 close

short depth, ctrlX, ctrlY;  //depth: pwm for moving vertical. 
char fLStatus = 1;          
char bLStatus = 1;          
short motorX = 0;           //pwm for motor 1, 3
short motorY = 0;           //pwm for motor 0, 2
short horRot = 0;           //y axis rotate, mapping accroding to the joystick input

//Default start up pwm of brushless is 1460, +- 30 deadband
//Max 1900, Min 1000
//angle bewteen brushless and boundary is 45 deg, assume -45 rotation for the coordinate
short pwm[4];
short sendPwm[8] = {1450, 1450, 1450, 1450, 1450, 1450, 1450, 1450};
short test = 0;

byte sendStatus1 = 0; 
byte sendStatus2 = 0;
byte infoCount = 0;
byte cmd[12]; //22 byte for the cmd

String temp;
String pressure; 

int lol = 0;

void setup() {
  Serial.begin(9600);
  Serial2.begin(28800);
  
  delay(500); //recommended delay for PS2 module start up
  ps2Err = ps2x.config_gamepad(PS2_CLK, PS2_CMD, PS2_SEL, PS2_DAT, pressures, rumble);
  if(ps2Err == 1)
    Serial.println("No controller found! Check your connection");
  else if(ps2Err == 2)
    Serial.println("Controller found but not accepting commands");
  tim1.every(100, tim1Event);
  tim2.every(1000, tim2Event);
}

void allStop(void){
  gM = 2; //move it upward 
  grip = 1; //stop
  clrMotor = 0; //stop
  fLStatus = 1; //level 1 brightness
  bLStatus = 1; //level 1 brightness
  pumpCtrl = 0; //stop
  for(char i=0; i<8; i++)
    sendPwm[i] = 1450;
  sendCmd();
}

//Tested before, not effective
void motorChange(short curPwm, short sendPwm, char mNum){
  #ifdef decel
  short temp, quo, rem; 
  
  if(abs(curPwm - sendPwm) > 600){
    #define stepNum (100)
    /*
    if(curPwm > sendPwm){
      temp = curPwm - sendPwm;
      quo = floor(temp/stepNum);
      rem = temp%stepNum;
      for (char i=0; i<quo; i++){
        sendPwm += stepNum;
        delay(1);
        motor[mNum].writeMicroseconds(sendPwm);
        if(i+1==quo){
          delay(1);
          motor[mNum].writeMicroseconds(sendPwm+rem);
        }
      }
    }
    else{
      temp = sendPwm - curPwm;
      quo = floor(temp/stepNum);
      rem = temp%stepNum;
      for (char i=0; i<quo; i++){
        sendPwm -= stepNum;
        delay(1);
        motor[mNum].writeMicroseconds(sendPwm);
        if(i+1==quo){
          delay(1);
          motor[mNum].writeMicroseconds(sendPwm-rem);
        }
      }
    }
    */
    
  }
  else
    motor[mNum].writeMicroseconds(curPwm);
  #else
    motor[mNum].writeMicroseconds(curPwm);
  #endif
}

void tim1Event(){
  if(ps2Err != 0){
    allStop();
    return;
  }
  else{   
    ps2x.read_gamepad();
    rotL = 0;
    rotR = 0;
    
    if(ps2x.ButtonPressed(PSB_START)){
      locked=!locked;
    }
    
    if(!locked){
      //Brushless Motor Related Control
      
      if(ps2x.Button(PSB_R1)){
        delay(1);
        if(ps2x.Button(PSB_R1)){
          rotR = 1;
          rotL = 0;
        }
      }
      else if(ps2x.Button(PSB_L1)){
        delay(1);
        if(ps2x.Button(PSB_L1)){
          rotR = 0;
          rotL = 1;
        }
      }

      if(ps2x.ButtonReleased(PSB_SELECT)){
        invCtrl = !invCtrl;
      }
    
      //Gripper Related Control
      if(ps2x.Button(PSB_PAD_UP)){
        delay(1); //To prevent bad connection
        if(ps2x.Button(PSB_PAD_UP))
          gM = 2;
      }
      else if(ps2x.Button(PSB_PAD_DOWN)){
        delay(1); //To prevent bad connection
        if(ps2x.Button(PSB_PAD_DOWN))
          gM = 0;
      }
      else
        gM = 1;
      
      if(ps2x.Button(PSB_L2)){
        delay(1);
        if(ps2x.Button(PSB_L2))
          grip = 0;
      }
      else if(ps2x.Button(PSB_R2)){
        delay(1);
        if(ps2x.Button(PSB_R2))
          grip = 2;
        
      }
      else
        grip = 1;
        
    
      //Clearing Motor 
      if(ps2x.ButtonReleased(PSB_CIRCLE)){
        clrMotor = !clrMotor;
      }
    
      //Lights Related Control
      if(ps2x.ButtonReleased(PSB_TRIANGLE)){
        lightSel = (~lightSel)&(B00000001);
      }
      if(ps2x.ButtonReleased(PSB_PAD_LEFT)){
        light[lightSel] -= 1;
        if(light[lightSel]<0)
          light[lightSel] = 0;
      }
      else if(ps2x.ButtonReleased(PSB_PAD_RIGHT)){
        light[lightSel] += 1;
        if(light[lightSel]>3)
          light[lightSel] = 3;
      }
    /*  switch(light[lightSel]){
        case 0:
          if(lightSel==0)
            fLStatus = 0;
          else
            bLStatus = 0;
          break;
        case 1:
          if(lightSel==0)
            fLStatus = 1;
          else
            bLStatus = 1;
          break;
        case 2:
          if(lightSel==0)
            fLStatus = 2;
          else
            bLStatus = 2;
          break;
        case 3:
          if(lightSel==0)
            fLStatus = 3;
          else
            bLStatus = 3;
          break;
      } */
          
      if(lightSel==0){
        fLStatus =  light[lightSel] ;
      }else{
        bLStatus =  light[lightSel] ;
      }

      //Pump control
      if(ps2x.ButtonReleased(PSB_SQUARE)){
        pumpCtrl += 1;
        if(light[lightSel]>0)
          pumpCtrl = 0;
      }
 
      ctrlX = map(ps2x.Analog(PSS_RX), 0, 255, 100, -100);
      ctrlY = map(ps2x.Analog(PSS_RY), 0, 255, -100, 100);
      //still need to add rotation
      motorX = map(ctrlX*cos(pi/4) - ctrlY*sin(pi/4),-141,141, 1900, 1000);
      motorY = map(ctrlX*sin(pi/4) + ctrlY*cos(pi/4), -141, 141, 1900, 1000);
      motorY = map(motorY, 1000, 1900, 1900, 1000);
      depth = map(ps2x.Analog(PSS_LY), 0, 255, 1000, 1900);
      horRot = map(ps2x.Analog(PSS_LX), 0, 255, 1900, 1000);
      
      pwm[0] = motorX + 100;
      pwm[1] = motorX - 100;
      pwm[2] = motorY + 100;
      pwm[3] = motorY - 100;
      for(char i=0; i<4; i++){
        if(pwm[i] > 1900)
          pwm[i] = 1900;
        else if(pwm[i] < 1000)
          pwm[i] = 1000;
      }

      if(invCtrl==0){
        if(rotR == 1){
         /* motorChange(pwm[0], sendPwm[0], 0);
          motorChange(pwm[2], sendPwm[1], 1);
          motorChange(pwm[1], sendPwm[2], 2);
          motorChange(pwm[3], sendPwm[3], 3);*/

          pwm[2] += 200;
          pwm[1] -= 200;
          
          for(char i=0; i<4; i++){
            if(pwm[i] > 1900)
              pwm[i] = 1900;
            else if(pwm[i] < 1000)
              pwm[i] = 1000;
          }

          
          sendPwm[0] = pwm[0];
          sendPwm[1] = pwm[2];
          sendPwm[2] = pwm[1];
          sendPwm[3] = pwm[3];
        }
        else if(rotL == 1){
         /*motorChange(pwm[1], sendPwm[0], 0);
          motorChange(pwm[3], sendPwm[1], 1);
          motorChange(pwm[0], sendPwm[2], 2);
          motorChange(pwm[2], sendPwm[3], 3);*/

          pwm[3] -= 200;
          pwm[0] += 200;

          for(char i=0; i<4; i++){
            if(pwm[i] > 1900)
              pwm[i] = 1900;
            else if(pwm[i] < 1000)
              pwm[i] = 1000;
          }
          
          sendPwm[0] = pwm[1];
          sendPwm[1] = pwm[3];
          sendPwm[2] = pwm[0];
          sendPwm[3] = pwm[2];
        }
        else{
          /*motorChange(motorX, sendPwm[0], 0);
          motorChange(motorX, sendPwm[1], 1);
          motorChange(motorY, sendPwm[2], 2);
          motorChange(motorY, sendPwm[3], 3);*/
          sendPwm[0] = motorX;
          sendPwm[1] = motorX;
          sendPwm[2] = motorY;
          sendPwm[3] = motorY;
        }
      }
      else{
        if(rotR == 1){
        /*  motorChange(map(pwm[0], 1000, 1900, 1900, 1000), map(sendPwm[0], 1000, 1900, 1900, 1000), 0);
          motorChange(pwm[2], sendPwm[1], 1);
          motorChange(pwm[1], sendPwm[2], 2);
          motorChange(pwm[3], sendPwm[3], 3);*/
          sendPwm[0] = pwm[0];
          sendPwm[1] = pwm[2];
          sendPwm[2] = pwm[1];
          sendPwm[3] = pwm[3];
        }
        else if(rotL == 1){
          /*motorChange(pwm[1], sendPwm[0], 0);
          motorChange(pwm[3], sendPwm[1], 1);
          motorChange(pwm[0], sendPwm[2], 2);
          motorChange(pwm[2], sendPwm[3], 3);*/
          sendPwm[0] = pwm[1];
          sendPwm[1] = pwm[3];
          sendPwm[2] = pwm[0];
          sendPwm[3] = pwm[2];
        }
        else{
          /*motorChange(map(motorX, 1000, 1900, 1900, 1000), sendPwm[0], 0);
          motorChange(map(motorX, 1000, 1900, 1900, 1000), sendPwm[1], 1);
          motorChange(map(motorY, 1000, 1900, 1900, 1000), sendPwm[2], 2);
          motorChange(map(motorY, 1000, 1900, 1900, 1000), sendPwm[3], 3);*/
          sendPwm[0] = map(motorX, 1000, 1900, 1900, 1000);
          sendPwm[1] = map(motorX, 1000, 1900, 1900, 1000);
          sendPwm[2] = map(motorY, 1000, 1900, 1900, 1000);
          sendPwm[3] = map(motorY, 1000, 1900, 1900, 1000);
        }
      }
      
      char rotOffset[2];
      if(horRot > 1600){
        rotOffset[0] = 200;
        rotOffset[1] = -200;
      }
      else if(horRot < 1300){
        rotOffset[0] = -200;
        rotOffset[1] = 200;
      }
      else{
        rotOffset[0] = 0;
        rotOffset[1] = 0;
      }
      motor[4].writeMicroseconds(depth + rotOffset[0]);
      motor[5].writeMicroseconds(depth + rotOffset[0]);
      motor[6].writeMicroseconds(depth + rotOffset[1]);
      motor[7].writeMicroseconds(depth + rotOffset[1]);

      sendPwm[4] = depth + rotOffset[0];
      sendPwm[5] = depth + rotOffset[0];
      sendPwm[6] = depth + rotOffset[1];
      sendPwm[7] = depth + rotOffset[1];
    }    
    sendCmd(); 
  }
}

void tim2Event(){
  //Status Report
  if(ps2Err != 0){
    Serial.println("Error! Trying to reconnect the PS2 controller....");
    ps2Err = ps2x.config_gamepad(PS2_CLK, PS2_CMD, PS2_SEL, PS2_DAT, pressures, rumble);
    allStop();
  }
  else{
    if(locked == 1){
      #ifdef debug
      Serial.println("Controller is locked! Press START to unlock");
      #endif
      allStop();
    }
    #ifdef debug
    Serial.println("Status Report: ");
    Serial.print("Clearning Motor: ");
    Serial.println(clrMotor, DEC);
    Serial.print("Front Light: ");
    Serial.println(fLStatus, DEC);
    Serial.print("Back Light: ");
    Serial.println(bLStatus, DEC);
    Serial.print("Light Selected: ");
    Serial.println(lightSel, DEC);
    Serial.print("L_Stick: ");
    Serial.println(depth, DEC);
    Serial.println("R_Stick: ");
    Serial.print(ctrlX, DEC);
    Serial.print(", ");
    Serial.println(ctrlY, DEC);
    Serial.print("gM: ");
    Serial.println(gM, DEC);
    Serial.print("clrMotor: ");
    Serial.println(clrMotor, DEC);
    Serial.print("grip: ");
    Serial.println(grip, DEC);
    Serial.print("pumpCtrl: ");
    Serial.println(pumpCtrl);
    Serial.print("motorX: ");
    Serial.println(motorX, DEC);
    Serial.print("motorY: ");
    Serial.println(motorY, DEC);
    Serial.print("pwm: ");
    Serial.print(pwm[0], DEC);
    Serial.print(", ");
    Serial.print(pwm[1], DEC);
    Serial.print(", ");
    Serial.print(pwm[2], DEC);
    Serial.print(", ");
    Serial.println(pwm[3], DEC);
    Serial.print("rotL: ");
    Serial.println(rotL, DEC);
    Serial.print("rotR: ");
    Serial.println(rotR, DEC);
    Serial.println("");
    Serial.println("");
    #endif
    Serial.print("pressure: ");
    Serial.println(pressure);
    Serial.print("Temp: ");
    Serial.println(temp);
  } 
}

void sendCmd(void){
  infoCount++;

  if(infoCount != 15){
    sendStatus1 = 0;
    sendStatus2 = 0;
    sendStatus1 = sendStatus1 | bLStatus;
    sendStatus1 = sendStatus1 | (fLStatus << 2);
    sendStatus1 = sendStatus1 | (grip << 4);
    sendStatus1 = sendStatus1 | (pumpCtrl << 6);
    sendStatus2 = sendStatus2 | gM;
    sendStatus2 = sendStatus2 | (clrMotor << 2);

    cmd[0] = 140;
    cmd[1] = sendStatus1;
    cmd[2] = sendStatus2;
    cmd[3] = map(sendPwm[0], 1000, 1900, 0, 255);
    cmd[4] = map(sendPwm[1], 1000, 1900, 0, 255);
    cmd[5] = map(sendPwm[2], 1000, 1900, 0, 255);
    cmd[6] = map(sendPwm[3], 1000, 1900, 0, 255);
    cmd[7] = map(sendPwm[4], 1000, 1900, 0, 255);
    cmd[8] = map(sendPwm[5], 1000, 1900, 0, 255);
    cmd[9] = map(sendPwm[6], 1000, 1900, 0, 255);
    cmd[10] = map(sendPwm[7], 1000, 1900, 0, 255);
    cmd[11] = 150;

    //confirm no data is same as the opening/ending sequence
    for(char i=1; i<11; i++){
      if(cmd[i] == 140)
        cmd[i] -= 1;
      else if(cmd[i] == 150)
        cmd[i] -= 1;
    }
    
    Serial2.write(cmd, 12);
    Serial2.flush(); //wait for all data inside tx buffer to be sent
  }
  else{
    //byte oldCmd[12];

    //for(char i=0; i<12; i++)
      //oldCmd[i] = cmd[i];
    
    infoCount = 0;
    cmd[0] = 140;
    cmd[1] = 255;
    cmd[2] = 255;
    cmd[3] = 127;
    cmd[4] = 127;
    cmd[5] = 127;
    cmd[6] = 127;
    cmd[7] = 127;
    cmd[8] = 127;
    cmd[9] = 127;
    cmd[10] = 127;
    cmd[11] = 150;

    Serial2.write(cmd, 12);
    #define DelayValue (60)
    delay(DelayValue);
    
    if(Serial2.available()){
      pressure = Serial2.readStringUntil('@');
      temp = Serial2.readStringUntil('!');
    }
  }
  
}

void loop() {
  tim1.update();
  tim2.update();
}

