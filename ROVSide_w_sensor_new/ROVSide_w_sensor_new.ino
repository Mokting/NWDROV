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

#define debug

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
short pumpPwm = 1450;
byte clrMotor = 0;
unsigned long ticks = 0;
unsigned long old_ticks = 0;
unsigned long offset = 0;
bool received = false;
byte fLstatus = 1;
byte grip = 1;
byte gM = 1;
byte buff[33]; //cmd from onshore board has 22 byte, check one by one
byte cmd[11];
byte lastcmd[11];

short ewma(short input, short past, float alpha) {
  return alpha * past + (1 - alpha) * input;
}

void sendData(void) {
  Serial.println("Sending");
  byte sendBuff[] = {(analogRead(pressPin)>>2)&0xFF};// - 96) * 1120 * 10197 / 102400000)
  Serial3.write(sendBuff, 1);
  Serial3.flush();
}


void cleanSpeed(void) {
  switch (clrSpeed) {
    case 1: {
        pumpPwm = ewma(1380, pumpPwm, 0.9);
        break;
      }
    case 2: {
        pumpPwm = ewma(1300, pumpPwm, 0.9);
        break;
      }
    default : {
        pumpPwm = ewma(1450, pumpPwm, 0.9);
        break;
      }
  }
  pump.writeMicroseconds(pumpPwm);
}


void allStop(void) {
  Serial.println("all stop");
  gripper.writeMicroseconds(1500);

  digitalWrite(gDirPin, HIGH);
  digitalWrite(clrMotorPin, HIGH);

  pump.writeMicroseconds(1450);
  fLight.write(1900);
  bLight.write(1900);
  for (char i = 0; i < 8; i++)
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

  for (char i = 0; i < 8; i++) {
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
  delay(10);
  manPwm.every(10, cleanSpeed);
}

void loop() {
  // tim1.update();
  ticks = millis();
  manPwm.update();
  if (ticks != old_ticks) {
    if (ticks % 150 == 0) {
      cmdReceive();
#ifdef debug
      if (gM != 1 || grip != 1 || fLstatus != 1 || clrSpeed != 0) {
        /*  Serial.print("Time:");
          Serial.print(millis(),DEC);
          Serial.println("\n");
          Serial.print("light: ");
          Serial.println(fLstatus);
          Serial.print("grip: ");
          Serial.println(grip);
          Serial.print("gM: ");
          Serial.println(gM);
          Serial.print("clrSpeed: ");
          Serial.println(clrSpeed);
          Serial.print("Motor: ");
          for(char i=0; i<8; i++){
          Serial.print(pwm[i]);
          Serial.print(", ");
          if(i==7)
          Serial.println("\n");}*/
        //  Serial.print("start of the buffer \n\n");
        //  for(int i=0;i<33;i++){
        //    Serial.println(buff[i],BIN);
        //    }
        //   Serial.println("end of the buffer");
        //   for(int i=0;i<11;i++){
        //    Serial.println(cmd[i]);
        //     }Serial.println("end of the cmd \n");
        //Serial.println(sendStatus1,BIN);

      }
#endif


    }
  }
  old_ticks = ticks;
}

void cmdExecute(void) {
  //cmd decode

  if (sendStatus1 == 252) {
    sendData();
  } else {
    fLstatus = sendStatus1 & (B00000011);
    grip = (sendStatus1 >> 2) & (B00000011);
    clrSpeed = (sendStatus1 >> 6) & (B00000011);
    gM = (sendStatus1 >> 4) & (B00000011);
    if (grip == 3 || clrSpeed == 3 || gM == 3) {
      return;
    }

  }

  // gripper motor control
  switch (gM) {
    case 0: {
        digitalWrite(gMotorPin, LOW);
        digitalWrite(gDirPin, HIGH);
        break;
      }
    case 1: {
        digitalWrite(gMotorPin, HIGH);
        digitalWrite(gDirPin, HIGH);
        break;
      }
    case 2: {
        digitalWrite(gMotorPin, LOW);
        digitalWrite(gDirPin, LOW);
        break;
      }
  }

  // gripper control
  switch (grip) {
    case 0: {
        gripper.writeMicroseconds(1700);
        break;
      }
    case 1: {
        gripper.writeMicroseconds(1500);
        break;
      }
    case 2: {
        gripper.writeMicroseconds(1100);
        break;
      }
  }

  // light cintrol
  switch (fLstatus) {
    case 0: {
        fLight.writeMicroseconds(1100);
        bLight.writeMicroseconds(1100);
        break;
      }
    case 1: {
        fLight.writeMicroseconds(1300);
        bLight.writeMicroseconds(1500);
        break;
      }
    case 2: {
        fLight.writeMicroseconds(1600);
        bLight.writeMicroseconds(1700);
        break;
      }
    case 3: {
        fLight.writeMicroseconds(1900);
        bLight.writeMicroseconds(1900);
        break;
      }
  }

  // send PWM to all thruster
  for (char i = 0; i < 8; i++) {
    motor[i].writeMicroseconds(pwm[i]);

  }
  /*Serial.println(millis(),DEC);
    Serial.println(pwm[4],DEC);
    Serial.println(pwm[5],DEC);
    Serial.println(pwm[6],DEC);
    Serial.println(pwm[7],DEC);*/

  if (pwm[4] >= 1600 || pwm[4] <= 1300) {
    for (int i = 0; i < 11; i++) {
      Serial.println(cmd[i]);
    }

  }

}

//Very on0 command handling function. Will integrate the newly written communcation code once i come back
void cmdReceive(void) {

  byte temp = 0;
  int start_time = 0;
  if (Serial3.available()) {

    while (Serial3.available()) {
      Serial3.readBytes(&temp, 1);
      for (byte i = 0; i < 32; i++) {
        buff[i] = buff[i + 1];
      }
      buff[32] = temp;
      for (byte i = 0; i < 11; i++) {
        cmd[i] = (((buff[i * 3] & buff[i * 3 + 1]) | (buff[i * 3] & buff[i * 3 + 2]) | (buff[i * 3 + 1] & buff[i * 3 + 2])) & 0xFF);
      }
      if (cmd[0] == 255 && cmd[10] == 235) {
        for (byte i = 0; i < 32; i++) {
          buff[i] = 0;
        }
        break;
      }
    }
    //Serial3.readBytes(buff, sizeof(buff)); //correct

    if (cmd[0] == 255 && cmd[10] == 235) {
      for (byte i = 0; i < 11; i++) {
        lastcmd[i] = cmd[i];
      }
      handler = 0;
    } else {
      Serial.println("Unable to decode");
      handler = 1;

    }


  } else {
    Serial.println("Missed!");
    handler = 2;
  }

  switch (handler) {
    case 0: {
        sendStatus1 = cmd[1];
        for (int i = 2; i < 11; i++) {
          pwm[i - 2] = map(cmd[i], 0, 255, 1000, 1900);
        }
        cmdExecute();
        if ((millis() - 1000) > start_time) {
          start_time = millis();
          on9 = 0;
        }
        break;
      }
    case 1: {
        on9++;
        if (on9 < 3) {
          for (byte i = 0; i < 11; i++) {
            //cmd[i] = lastcmd[i];
          }
          //if(cmd[1]==252){cmd[1]=255;}
          //cmdExecute();
          /*  Serial.println("end of the buffer");
            for(int i=0;i<11;i++){
             Serial.println(cmd[i]);
             }Serial.println("end of the cmd \n");
             break;
            }*/
          break;
        }
      }
    case 2: {
        allStop();
        received = false;
        offset = 0;
        byte discard;
        while (Serial3.available()) {
          Serial3.readBytes(&discard, 1);
        }
        for (byte i = 0; i < 32; i++) {
          buff[i] = 0;
        }
        break;
      }
  }

}






