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
#define root_1_2 (0.707107)

#define debug         0
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
//char lightSel   = 1;        //0 front, 1 black
char light   = 0;    //4 state: Off, brightness 1, 2, 3
bool clrMotor   = false;        //clearing motor: On, Off
char clrSpeed = 0;          //pump control: 0 Off, 1 On. Useless now as the pump will on together with clearning motor
bool invCtrl = false;           //0 Gripper as front, 1 clearing motor as front
char gM = 1;                //gripper motor: 0 down, 1 off, 2 up
char grip = 1;              //grippe: 0 open. 1 stop, 2 close

short depth, ctrlX, ctrlY;  //depth: pwm for moving vertical. 
//char fLStatus = 0;          
//char bLStatus = 0;          
short motorX = 0;           //pwm for motor 1, 3
short motorY = 0;           //pwm for motor 0, 2
short horRot = 0;           //y axis rotate, mapping accroding to the joystick input
float TestTest = 0.0;
//Default start up pwm of brushless is 1460, +- 30 deadband
//Max 1900, Min 1000
//angle bewteen brushless and boundary is 45 deg, assume -45 rotation for the coordinate
short pwm[4];
short sendPwm[8] = {1450, 1450, 1450, 1450, 1450, 1450, 1450, 1450};
short test = 0;
short pastIn[8] = {1450, 1450, 1450, 1450, 1450, 1450, 1450, 1450};

byte sendStatus1 = 0; 
byte receive_counter = 0;
byte infoCount = 0;
byte cmd[11]; //22 byte for the `
byte buff[3];

byte realcmd[33];
byte locktime = 0;
bool received = false;
String temp;
String pressure; 

byte Depth = 0;
int lol = 0;

byte keyPress = 0;
int Tested = 0;
unsigned long ticks = 0;
unsigned long old_ticks = 0;

void setup() {
  Serial.begin(9600);
  Serial2.begin(28800);
  //Serial.setTimeout(10);
  delay(500); //recommended delay for PS2 module start up
  
  ps2Err = ps2x.config_gamepad(PS2_CLK, PS2_CMD, PS2_SEL, PS2_DAT, pressures, rumble);
  
  if(ps2Err == 1)
    Serial.println("No controller found! Check your connection");
  else if(ps2Err == 2)
    Serial.println("Controller found but not accepting commands");
  
 // tim1.every(100, tim1Event);
 // tim2.every(103, tim2Event);
}

void allStop(void){
  gM = 1; //move it upward 
  grip = 1; //stop
  //clrMotor = false; //stop
  light= 1; 
  clrSpeed = 0; //stop
  for(char i=0; i<8; i++){
    sendPwm[i] = 1450;
    pastIn[i] = 1450;}
  sendCmd();
  
}

//function for reading ps2 controller command and sending command. Execute every 100ms
void tim1Event(){
  if(ps2Err != 0){
    allStop();
    //return;
  }
  else{  
    ps2x.read_gamepad();
    rotL = 0;
    rotR = 0;
   
    if(ps2x.ButtonPressed(PSB_START)){
      keyPress |= 0x01;
      locked = !locked;
    }
    
    //if the controller is not locker, read any input from the controller and send command
    if(!locked){
      //Brushless Motor Related Control  
      if(ps2x.Button(PSB_R1)){
        delay(1);
        if(ps2x.Button(PSB_R1)){
          if(!invCtrl){
            rotR = 1;
            rotL = 0;
          }
          else{
            rotR = 0;
            rotL = 1;
          }
        }
      }
      else if(ps2x.Button(PSB_L1)){
        delay(1);
        if(ps2x.Button(PSB_L1)){
          if(!invCtrl){
            rotR = 0;
            rotL = 1;
          }
          else{
            rotL = 0;
            rotR = 1;
          }
        }
      }

      if(ps2x.ButtonReleased(PSB_SELECT)){
        keyPress |= 0x02;
        invCtrl = !invCtrl;
      }
    
      //Gripper Related Control
      if(ps2x.Button(PSB_PAD_UP)){
        delay(1); //To prevent bad connection
        if(ps2x.Button(PSB_PAD_UP))
          gM = 2;
          keyPress |= 0x04;
      }
      else if(ps2x.Button(PSB_PAD_DOWN)){
        delay(1); //To prevent bad connection
        if(ps2x.Button(PSB_PAD_DOWN))
          gM = 0;
          keyPress |= 0x08;
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
     //   clrMotor = !clrMotor;
      }
    
      //Lights Related Control
      // They said don't need to control the light seperately
      /*if(ps2x.ButtonReleased(PSB_TRIANGLE)){
        lightSel += 1;
        if(lightSel>1)
          lightSel = 0;
      }*/
      if(ps2x.ButtonReleased(PSB_PAD_LEFT)){
        light -= 1;
        if(light<0)
          light = 0;
      }
      else if(ps2x.ButtonReleased(PSB_PAD_RIGHT)){
        light += 1;
        if(light>3)
         light = 3;
      }

      //Pump control
      if(ps2x.ButtonReleased(PSB_SQUARE)){
        clrSpeed += 1;
        if(clrSpeed > 2)
          clrSpeed = 0;
      }

      ctrlX = map(ps2x.Analog(PSS_RX), 0, 255, 25, -25); //maybe this one can be lower, e.g. 100->80 for lower rotation rate
      ctrlY = map(ps2x.Analog(PSS_RY), 0, 255, -90, 90); 
      
      depth = map(ps2x.Analog(PSS_LY), 0, 255, 1270, 1630);
    //  horRot = map(ps2x.Analog(PSS_LX), 0, 255, 1900, 1000);
      motorX = map(ctrlX*root_1_2 - ctrlY*root_1_2, -141,141, 1900, 1000);
      motorY = map(ctrlX*root_1_2 + ctrlY*root_1_2, -141, 141, 1000, 1900);

      //temp pwm value for left/right movement, code is on9 since this is not originally designed for this purpose
      pwm[0] = motorX + 150;
      pwm[1] = motorX - 150;
      pwm[2] = motorY + 150;
      pwm[3] = motorY - 150;
      

      if(invCtrl==0){
        if(rotR == 1){
          pwm[2] += (-320);
          pwm[1] -= (-320);
          
          sendPwm[0] = pwm[0];
          sendPwm[1] = pwm[2];
          sendPwm[2] = pwm[1];
          sendPwm[3] = pwm[3];
        }
        else if(rotL == 1){
          pwm[3] -= (-320);
          pwm[0] += (-320);
          
          sendPwm[0] = pwm[1];
          sendPwm[1] = pwm[3];
          sendPwm[2] = pwm[0];
          sendPwm[3] = pwm[2];
        }
        else{
          sendPwm[0] = motorX;
          sendPwm[1] = motorX;
          sendPwm[2] = motorY;
          sendPwm[3] = motorY;
        }
      }
      else{
        if(rotR == 1){
          pwm[2] += (-320);
          pwm[1] -= (-320);
          
          sendPwm[0] = pwm[0];
          sendPwm[1] = pwm[2];
          sendPwm[2] = pwm[1];
          sendPwm[3] = pwm[3];
        }
        else if(rotL == 1){
          pwm[3] -= (-320);
          pwm[0] += (-320);

          
          sendPwm[0] = pwm[1];
          sendPwm[1] = pwm[3];
          sendPwm[2] = pwm[0];
          sendPwm[3] = pwm[2];
        }
        else{
          sendPwm[2] = map(motorX, 1000, 1900, 1900, 1000);
          sendPwm[3] = map(motorX, 1000, 1900, 1900, 1000);
          sendPwm[0] = map(motorY, 1000, 1900, 1900, 1000);
          sendPwm[1] = map(motorY, 1000, 1900, 1900, 1000);
        }
      }

      sendPwm[4] = depth;
      sendPwm[5] = depth;
      sendPwm[6] = depth;
      sendPwm[7] = depth;

      
      for(byte i=0; i<8; i++){
        if(sendPwm[i]>1900){sendPwm[i]=1900;}
        if(sendPwm[i]<1000){sendPwm[i]=1000;}
        if(i<4){
          sendPwm[i] = ewma(sendPwm[i], pastIn[i], 0.9);
        }
        else{
          sendPwm[i] = ewma(sendPwm[i], pastIn[i], 0.96);
        }
        pastIn[i] = sendPwm[i];
      }
      sendCmd(); 
    }else if(locked){
      allStop();
      }    
    
  }
}

void tim2Event(){
  //Status Report
  if(ps2Err != 0){
    Serial.println("Error! Trying to reconnect the PS2 controller....");
    ps2Err = ps2x.config_gamepad(PS2_CLK, PS2_CMD, PS2_SEL, PS2_DAT, pressures, rumble);
    //allStop();
  }
  else{
    #ifdef debug
   // Serial.println("Status Report: ");
   /* Serial.print("Clearning Motor: ");
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
    Serial.print("clrSpeed: ");
    Serial.println(clrSpeed);
    Serial.print("motorX: ");
    Serial.println(motorX, DEC);
    Serial.print("motorY: ");
    Serial.println(motorY, DEC);
    Serial.print("pwm: ");
    Serial.print(sendPwm[0], DEC);
    Serial.print(", ");
    Serial.print(sendPwm[1], DEC);
    Serial.print(", ");
    Serial.print(sendPwm[2], DEC);
    Serial.print(", ");
    Serial.println(sendPwm[3], DEC);
    Serial.print("rotL: ");
    Serial.println(rotL, DEC);
    Serial.print("rotR: ");
    Serial.println(rotR, DEC);
    Serial.println("");
    Serial.println("");*/
    #endif
    Serial.print("\nLocked: ");
    Serial.println(locked, DEC);
    Serial.print("Inverted: ");
    Serial.println(invCtrl, DEC);
    Serial.print("Depth: ");
    Serial.println(Depth, DEC);
    //Serial.println(keyPress,BIN);
   /* Serial.println(millis(),DEC);
    Serial.println(cmd[6],DEC);
    Serial.println(cmd[7],DEC);
    Serial.println(cmd[8],DEC);
    Serial.println(cmd[9],DEC);*/
    Serial.println("");
    /*if(temp=="O"){
      Serial.println("OK");
    }else{
      Serial.println("FUCKED");
      }*/
    //Serial.print("gM: ");
    //Serial.println(gM, DEC);
    /*for(int i=0;i<11;i++){
      Serial.println((cmd[i]),BIN);
      }*/
  } 
}

void sendCmd(){
  if(receive_counter < 15){
    sendStatus1 =  (light|(grip << 2)|(gM << 4)|(clrSpeed << 6))&(0xFF);
    // 11111100 = 252
    // repeatition code 
    receive_counter ++;
    
    cmd[0] = 255;
    cmd[1] = sendStatus1;
    cmd[2] = map(sendPwm[0], 1000, 1900, 0, 255);
    cmd[3] = map(sendPwm[1], 1000, 1900, 0, 255);
    cmd[4] = map(sendPwm[2], 1000, 1900, 0, 255);
    cmd[5] = map(sendPwm[3], 1000, 1900, 0, 255);
    cmd[6] = map(sendPwm[4], 1000, 1900, 0, 255);
    cmd[7] = map(sendPwm[5], 1000, 1900, 0, 255);
    cmd[8] = map(sendPwm[6], 1000, 1900, 0, 255);
    cmd[9] = map(sendPwm[7], 1000, 1900, 0, 255);
    cmd[10] = 235;
    
    //confirm no data is same as the opening/ending sequence
    for(char i=2; i<=9; i++){
       if(cmd[i] == 255)
        cmd[i]--;
        if(cmd[i]==235)
          cmd[i]--;
    }

    for(char i=0;i<33;i++){
      realcmd[i] = cmd[i/3];
      }

    
    Serial2.write(realcmd, 33);
    Serial2.flush(); //wait for all data inside tx buffer to be sent
  }else{
    //Serial.println("Function is called");
    receive_counter = 0;
    unsigned long timeout = millis();
    
    cmd[0] = 255;
    cmd[1] = 252;
    cmd[2] = map(sendPwm[0], 1000, 1900, 0, 255);
    cmd[3] = map(sendPwm[1], 1000, 1900, 0, 255);
    cmd[4] = map(sendPwm[2], 1000, 1900, 0, 255);
    cmd[5] = map(sendPwm[3], 1000, 1900, 0, 255);
    cmd[6] = map(sendPwm[4], 1000, 1900, 0, 255);
    cmd[7] = map(sendPwm[5], 1000, 1900, 0, 255);
    cmd[8] = map(sendPwm[6], 1000, 1900, 0, 255);
    cmd[9] = map(sendPwm[7], 1000, 1900, 0, 255);
    cmd[10] = 235;
    for(char i=0;i<33;i++){
     realcmd[i] = cmd[i/3];
    }
    
    Serial2.write(realcmd, 33);
    Serial2.flush(); //wait for all data inside tx buffer to be sent
     while((millis()-timeout <= 200)){
     byte temp;
     if(Serial2.available()){
      Serial2.readBytes(&temp,1);
      //Serial.println(millis(),DEC);
      //Serial.println("OKOK\n");
      Serial.println(temp<<2,DEC);
      Depth = (((temp<<2)-96) * 1120 * 10197 / 102400000);
      if(Depth<0){
         Depth = 0;
        }
        Serial.println(Depth,DEC);
      locktime = 0;
      byte discard;
      while(Serial2.available()){
       Serial2.readBytes(&discard, 1);
     }
      return;}
     }

     // Serial.println(millis(),DEC);
      locktime++;
    if(locktime>4){
      locktime = 5;
      locked = true;
    }
    
  }
/*   while(Serial2.available()){
     byte temp;
     Serial2.readBytes(&temp,1);
     for(byte i=0;i<2;i++){
       buff[i]=buff[i+1];
     }
        buff[2]=temp;
     if(buff[0]==255&&buff[2]==254){
      Serial.println("OKOK");
      Depth = buff[1];
      received = true;
      locktime = 0;
        break;
      }
      received = false;
   }

    
   if(!received){
    locktime++;
    if(locktime>5){
      locktime = 5;
      Serial.println("locktime exceed");
      Serial.println(buff[0]);
      Serial.println(buff[1]);
      Serial.println(buff[2]);
      locked = true;
    }
   }*/
}

//Expotential weighted moving average
short ewma(short input, short past, float alpha){
  return (short)(alpha * past + (1 - alpha) * input);
}

void loop() {
   ticks = millis();
   if(ticks!=old_ticks){
    if(ticks%100==0){
        tim1Event();
      }
     if(ticks%150==47){
        tim2Event();
      }
     old_ticks = ticks;
   }
}

