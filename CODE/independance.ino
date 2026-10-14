#include <VL6180X.h>
#include <VL53L0X.h>
#include <Wire.h>
//#include "piccolaPins.h"
#define PWMA PB9   //pb8X //motor driver pins
#define AIN2 PA10//PA12
#define AIN1 PA7
#define STBY PA12//PA11
#define BIN1 PB0
#define BIN2 PB1
#define PWMB PB8 //pb9X

#define leftOut1 PB12   //interrupt pins of motors 
#define leftOut2 PB13         
#define rightOut1 PB14
#define rightOut2 PB11 //pb11

#define led PC13 

void motorDiver()
{   
    pinMode(PWMA, OUTPUT);
    pinMode(AIN2, OUTPUT);
    pinMode(AIN1, OUTPUT);
    pinMode(STBY, OUTPUT);
    pinMode(BIN1, OUTPUT);
    pinMode(BIN2, OUTPUT);
    pinMode(PWMB, OUTPUT);
}

void motorInterrupt()
{
    pinMode(leftOut1, INPUT);
    pinMode(leftOut2, INPUT);
    pinMode(rightOut1, INPUT);
    pinMode(rightOut2, INPUT);
}

//#include "piccolaVariables.h"
int leftBase =170;
int rightBase = 170;
int leftPwm;
int rightPwm;

unsigned long leftEncoder = 0;
unsigned long rightEncoder = 0;
unsigned long encoderLeftCount = 0;
unsigned long encoderRightCount = 0;
int dif = 0;

float tof[5];
int wallAvailable[3];
int cellWalls[3];

int rightWall  = 1;
int leftWall = 1;

float leftP = 0.35;
float leftD = 0.8;

float leftError = 0;
float leftLastError = 0;

float rightP = 0.35;
float rightD = 0.8;

float rightError = 0;
float rightLastError = 0;

float wallP = 0.25;
float wallD = 1.8;

float wallError = 0;
float wallLastError = 0;

float correction;

char nextMove = 'F';
//char R1;
unsigned long time;


boolean left,right,front;
int cell =0;
boolean  L = true;
boolean  R= true;
boolean  F=   false; 

//#include "piccolaTof.h"
#define GPIO1 PA0         
#define GPIO2 PA1 //---------------         
#define GPIO3 PA5
#define GPIO4 PA6//--------
#define GPIO5 PA4

#define tofAddress1 43
#define tofAddress2 44 
#define tofAddress3 45
#define tofAddress4 46
#define tofAddress5 47

VL6180X tof1;
VL53L0X tof2;
VL6180X tof3;
VL53L0X tof4;
VL6180X tof5;


void tofSetup()
{
    pinMode(GPIO1, OUTPUT);
    pinMode(GPIO2, OUTPUT);
    pinMode(GPIO3, OUTPUT);
    pinMode(GPIO4, OUTPUT);
    pinMode(GPIO5, OUTPUT);
    digitalWrite(GPIO1, LOW);
    digitalWrite(GPIO2, LOW);
    digitalWrite(GPIO3, LOW);
    digitalWrite(GPIO4, LOW);
    digitalWrite(GPIO5, LOW);

    Wire.begin();

    digitalWrite(GPIO1, HIGH);
    delay(10);
    tof1.init();
    tof1.configureDefault();
    tof1.setTimeout(100);
    tof1.setAddress(tofAddress1);

     digitalWrite(GPIO2, HIGH);
    delay(10);
    tof2.init();
    tof2.startContinuous();
    //tof2.startInterleavedContinuous();
    tof2.setTimeout(100);
    tof2.setAddress(tofAddress2);
    tof[2]=tof2.readRangeContinuousMillimeters();




    digitalWrite(GPIO3, HIGH);
    delay(10);
    tof3.init();
    tof3.configureDefault();
    //tof2.startInterleavedContinuous();
    tof3.setTimeout(100);
    tof3.setAddress(tofAddress3);
    
    digitalWrite(GPIO4, HIGH);
    delay(10);
    tof4.init();
    tof4.startContinuous();
    //tof2.startInterleavedContinuous();
    tof4.setTimeout(100);
    tof4.setAddress(tofAddress4);
    tof[3]=tof4.readRangeContinuousMillimeters();


    digitalWrite(GPIO5, HIGH);
    delay(10);
    tof5.init();
    tof5.configureDefault();
    //Sensor1.startRangeContinuous();
    tof5.setTimeout(100);
    tof5.setAddress(tofAddress5);
}
void tofPid()
{
    tof[0] = tof1.readRangeSingleMillimeters();
    tof[2] = tof3.readRangeSingleMillimeters();
    tof[4] = tof5.readRangeSingleMillimeters();
    
}

void tofCell()
{

    tof[1] = tof2.readRangeContinuousMillimeters();
    tof[2] = tof3.readRangeSingleMillimeters();
    tof[3] = tof4.readRangeContinuousMillimeters();
    
    
}

void printTof()
{
    
    Serial2.print(tof[0]);
    Serial2.print(",  ");
    
    Serial2.print(tof[1]);
    Serial2.print(",  ");
    Serial2.print(tof[2]);
    Serial2.print(",  ");
    Serial2.print(tof[3]);
    Serial2.print(",  ");
    
    Serial2.print(tof[4]);
    Serial2.println();
    

    
}
void checkWallsPid()
{
    if (tof[2] > 180)
    {
        cellWalls[1] =  0;
    }
    else
    {
        cellWalls[1] =  1;
    } 


    if (tof[0] <= 170)
    {
        wallAvailable[0] = 1;
    }
    else
    {
        wallAvailable[0] = 0; 
    }

    if (tof[4] <= 170)
    {
        wallAvailable[2] = 1;
    }
    else
    {
        wallAvailable[2] = 0; 
    }
}
void checkWallsCell()
{
    if (tof[2] > 180)
    {
        cellWalls[1] =  0;
    }
    else
    {
        cellWalls[1] =  1;
    } 

    if (tof[1] <= 190)
    {
        cellWalls[0] = 1;
    }
    else
    {
        cellWalls[0] = 0; 
    }

    if (tof[3] <= 280 )
    {
        cellWalls[2] = 1;
    }
    else
    {
        cellWalls[2] = 0; 
    }


}


//#include "piccolaMotors.h"
void stbyHigh()
{
    digitalWrite(STBY, HIGH);
}

void stbyLow()
{
    digitalWrite(STBY, LOW);
}

void leftForward()
{
    digitalWrite(AIN1, HIGH);
    digitalWrite(AIN2, LOW);
}

void leftReverse()
{
    digitalWrite(AIN1, LOW);
    digitalWrite(AIN2, HIGH);
}

void leftBrake()
{
    digitalWrite(AIN1, HIGH);
    digitalWrite(AIN2, HIGH);
}

void leftForwardBase()
{
    stbyHigh();
    leftForward();
    analogWrite(PWMA, leftBase);
}

void leftReverseBase()
{
    stbyHigh();
    leftReverse();
    analogWrite(PWMA, leftBase);
}

void rightForward()
{
    digitalWrite(BIN1, LOW);
    digitalWrite(BIN2, HIGH);
}

void rightReverse()
{
    digitalWrite(BIN1, HIGH);
    digitalWrite(BIN2, LOW);
}

void rightBrake()
{
    digitalWrite(BIN1, HIGH);
    digitalWrite(BIN2, HIGH);
}

void rightForwardBase()
{
    stbyHigh();
    rightForward();
    analogWrite(PWMB, rightBase);
}

void rightReverseBase()
{
    stbyHigh();
    rightReverse();
    analogWrite(PWMB, rightBase);
}

void writePwm()
{
    analogWrite(PWMA, leftPwm);
    analogWrite(PWMB, rightPwm);
}

void writeBasePwm()
{
    analogWrite(PWMA, leftBase);
    analogWrite(PWMB, rightBase);
}

void forward()
{
    stbyHigh();
    leftForward();
    rightForward();
    writePwm();
}

void reverse()
{
    stbyHigh();
    leftReverse();
    rightReverse();
    writePwm();
}

void brake()
{
    stbyHigh();
    leftBrake();
    rightBrake();
    delay(100);
}

void forwardBase()
{
    stbyHigh();
    leftForward();
    rightForward();
    writeBasePwm();
}

void reverseBase()
{
    stbyHigh();
    leftReverse();
    rightReverse();
    writeBasePwm();
}

void turnRight()
{
    stbyHigh();
    leftReverse();
    rightForward();
    writeBasePwm();
}

void turnLeft()
{
    stbyHigh();
    leftForward();
    rightReverse();
    writeBasePwm();
}
void countLeftOut1()
{
    leftEncoder = leftEncoder + 1;  
     
}

void countRightOut1()
{
    rightEncoder = rightEncoder + 1;
}
void wallFollow()
{
  tofPid();
  
   checkWallsPid();
    if (wallAvailable[0] == 1 && wallAvailable[2] == 1)
    {
        wallPid();
      // rightPid();
     //  leftPid();
       forward();
    }

    else if (wallAvailable[0] == 0 && wallAvailable[2] == 1)
    {
        rightPid();
       // brake();
        forward();
    }

    else if (wallAvailable[0] == 1 && wallAvailable[2] == 0)
    {
        leftPid();
       // brake();
        forward();
    }
    else if(wallAvailable[0] == 0 && wallAvailable[2] == 0)
    {
       
        leftPid(); 
        rightPid();
        forward();
       //
       forwardBase();

      //  brake();

    }
}  
void wallPid()
{
    wallError = (tof[0] - tof[4]);
    rightLastError = 90 - tof[4];
    leftLastError = 90 - tof[0];
    correction = (wallError * wallP) + ((wallError - wallLastError) * wallD);
    wallLastError = wallError;

    if (correction > 30 )
    {
        correction = 0;
    }
    else if (correction < -30)
    {
        correction = 0;
    }

    leftPwm = leftBase - correction;
    rightPwm = rightBase + correction;

    // Serial print the PID correction values
    
}

void rightPid()
{
    rightError = 80 - tof[4];
    correction  = (rightError * rightP) + ((rightError - rightLastError) * rightD);
    rightLastError = rightError;

    if (correction > 30 )
    {
        correction = 0;
    }
    else if (correction < -30)
    {
        correction = 0;
    }

    leftPwm = leftBase - correction;
    rightPwm = rightBase + correction;

    // Serial print the PID correction values
    
}

void leftPid()
{
    leftError = 80 - tof[0];
    correction = (leftError * leftP) + ((leftError - leftLastError) * leftD);
    leftLastError = leftError;

    if (correction > 30 )
    {
        correction = 0;
    }
    else if (correction < -30)
    {
        correction = 0;
    }

    leftPwm = leftBase + correction;
    rightPwm = rightBase - correction;

    // Serial print the PID correction values
    Serial.print("Left PID Correction: ");
    Serial.print(correction);
    Serial.print(" | Left PWM: ");
    Serial.print(leftPwm);
    Serial.print(" | Right PWM: ");
    Serial.println(rightPwm);
}


void cellBrake()
{
  
        tof[2] = tof3.readRangeSingleMillimeters();

   while(tof[2]>=100)
    {   
        wallFollow();
    }
    brake();
    encoderLeftCount = leftEncoder;
    encoderRightCount = rightEncoder;
}


void RcellBrake()
{
   rightBase=150;
    leftBase=150;
        tof[4] = tof5.readRangeSingleMillimeters();
        tof[2] = tof3.readRangeSingleMillimeters();
        tof[0] = tof1.readRangeSingleMillimeters();

   while(tof[4]<160)
    {    
        if(tof[2]<90)
        {
        //  brake();
         // mazeSolve(); 
        // nextstep();
          if(tof[4]<=170 && tof[0]<=170)
        {
          turnBack();
        }
        else if(tof[4]>=170)
        {
          brake();
          turnRight();
          brake();
          delay(100);
          if(tof[2]<=90 && tof[4]>=170)
          {
             brake();
        leftAboutTurn();
             brake();
          }
        }
        else if (tof[0]>=170)
        {
          brake();
          leftAboutTurn();
        }
          wallFollow();
                }
        else
        {
          wallFollow();
        }

    }
  //  brake();
    checkWallsCell();
    if(cellWalls[1] == 0)
    {
     // leftPid();
      //rightPid();
    encoderLeftCount = leftEncoder;
    encoderRightCount = rightEncoder;
    
    encoderRightCount = encoderRightCount + 250;
    encoderLeftCount = encoderLeftCount + 250;

      

       while (rightEncoder <= encoderRightCount || leftEncoder <= encoderLeftCount)
      {
        if(tof[2]>200)
        {
       wallFollow();
        }
        else if(tof[2]<200)
        {
              encoderLeftCount = leftEncoder;
              encoderRightCount = rightEncoder;
              encoderRightCount = encoderRightCount + 200;
              encoderLeftCount = encoderLeftCount + 200;
               while (rightEncoder <= encoderRightCount || leftEncoder <= encoderLeftCount)
               {
                wallFollow();
               }

        }
      }
    }
 
   
    else 
    { tof[2] = tof3.readRangeSingleMillimeters();
   while(tof[2]>100)
    {    
       wallFollow();
    }   
      }
    brake();
    
   // delay(100);
}
void LcellBrake()
{            rightBase=150;
    leftBase=150;
        tof[0] = tof1.readRangeSingleMillimeters();    
            tof[4] = tof5.readRangeSingleMillimeters();
        tof[2] = tof3.readRangeSingleMillimeters();

   while(tof[0]<160)
    {
        if(tof[2]<120)
        {
    
         // mazeSolve(); 
        // nextstep();
          if(tof[4]<=170 && tof[0]<=170)
        {
          turnBack();
        }
        else if (tof[0]>=170)
        {
          leftAboutTurn();
        }
         wallFollow();
                }
        else
        {
          wallFollow();
        }
    }
    brake();
    checkWallsCell();

    if(cellWalls[1] == 0)
    {
      //rightPid();
     // leftPid();
    encoderLeftCount = leftEncoder;
    encoderRightCount = rightEncoder;
    
    encoderRightCount = encoderRightCount + 50;
    encoderLeftCount = encoderLeftCount + 50;

      while (rightEncoder <= encoderRightCount || leftEncoder <= encoderLeftCount)
      {
     
          wallFollow();
      }
    }
    
    else
    { tof[2] = tof3.readRangeSingleMillimeters();
      while(tof[2]>=105)
      {   
        wallFollow();
       }

    }
     brake();
}
void rightAboutTurn()
{
    encoderLeftCount = leftEncoder;
    encoderRightCount = rightEncoder;
    

   rightBase=170;
    leftBase=170;
    encoderLeftCount = 0;
    encoderRightCount = 0;
    leftEncoder = 0;
    rightEncoder = 0;
    encoderRightCount= encoderRightCount + 195;
    encoderLeftCount= encoderLeftCount + 200;
    while(rightEncoder <= encoderRightCount)
    {
        turnRight();
    }

    brake();
   rightBase=170;
    leftBase=170;  
      encoderLeftCount = 0;
    encoderRightCount = 0;
    leftEncoder = 0;
    rightEncoder = 0;
}
void leftAboutTurn()
{
  brake();
   rightBase=170;
    leftBase=170;
    encoderLeftCount = 0;
    encoderRightCount = 0;
    leftEncoder = 0;
    rightEncoder = 0;
    encoderRightCount= encoderRightCount + 225;
    encoderLeftCount= encoderLeftCount + 205;
    while(rightEncoder <= encoderRightCount)
    {
        turnLeft();
    }
    

    brake();
   rightBase=170;
    leftBase=170;
    encoderLeftCount = 0;
    encoderRightCount = 0;
    leftEncoder = 0;
    rightEncoder = 0;
}
void turnBack(){
     rightBase=170;
    leftBase=170;
    encoderRightCount= encoderRightCount + 450;
    encoderLeftCount= encoderLeftCount + 440;
    while(rightEncoder <= encoderRightCount)
    {
       turnLeft();
    }

    brake();
   rightBase=170;
    leftBase=170;
    
    encoderLeftCount = 0;
    encoderRightCount = 0;
    leftEncoder = 0;
    rightEncoder = 0;

}
void cellStart()
{
    /*encoderRightCount = encoderRightCount + 300;
    encoderLeftCount = encoderLeftCount + 300;
    rightBase=70;
    leftBase=70;
    while (rightEncoder <= encoderRightCount || leftEncoder <= encoderLeftCount)
    {
        dif = leftEncoder - encoderLeftCount + 300;
        rightBase = int(70+110/(1+pow(2.73,((150-dif)*0.05))));
        leftBase = int(70+110/(1+pow(2.73,((150-dif)*0.05))));
        forwardBase();
    }
    rightBase=210;
    leftBase=180;*/
   rightBase=170;
    leftBase=170;
    encoderRightCount= encoderRightCount + 50;
    encoderLeftCount= encoderLeftCount + 50;
    while (rightEncoder <= encoderRightCount || leftEncoder <= encoderLeftCount)
    {
     // wallPid();
       tofPid();
        wallFollow();
    }
    encoderLeftCount = leftEncoder;
    encoderRightCount = rightEncoder;
   brake();
    //delay(100);
}
void RcellStart()
{

     tof[4] = tof5.readRangeSingleMillimeters();
        //tof[2] = tof3.readRangeSingleMillimeters();
   while(tof[4]>160)
    {   
        wallFollow();
    }
   // brake();
   // delay(100);
}
void LcellStart()
{


     tof[0] = tof5.readRangeSingleMillimeters();
        //tof[2] = tof3.readRangeSingleMillimeters();
   while(tof[0]>160)
    {  

        wallFollow();
    }
    //brake();
   // delay(100);
}
void cellForward()
{
    encoderLeftCount = encoderLeftCount + 50;
    encoderRightCount = encoderRightCount + 50;
    while (leftEncoder <= encoderLeftCount || rightEncoder <= encoderRightCount)
    {
        tofCell();
     //  wallPid();
       wallFollow();
      //  forwardBase();
    }
    encoderLeftCount = leftEncoder;
    encoderRightCount = rightEncoder;
}

/*void rightTurn()
{
  cellBrake();
  rightAboutTurn();
  cellStart();
}*/
void rightTurn()
{
  RcellBrake();
  rightAboutTurn();
  RcellStart();

}
/*void leftTurn(){
  cellBrake();
  leftAboutTurn();
  cellStart();
}*/
void leftTurn(){
  LcellBrake();
  leftAboutTurn();
 LcellStart();

}
void cellBack()
{
    cellBrake();
    turnBack();
    cellStart();
}

void mazeSolve()
{
    wallFollow();
    tofCell();
    checkWallsCell();
    checkWallsPid();
    
    if (cellWalls[2] == 0 )
    {   

        nextMove = 'R';

    }

    else if (cellWalls[1] == 0)
    {
        nextMove = 'F';
      
    }

    else if (cellWalls[0] == 0)
    {

        nextMove = 'L';
      
    }
    else if (tof[2] <=180 && tof[0] <=160 && tof[4] <= 160 )
    {
       nextMove = 'B';
    
    }
    


    switch (nextMove)
    {
    case 'F' :      //move forward
        cellForward();
        break;
   /* case '>' :      //move right NO WALL IN F
       brake();
       NWrightTurn();
      */  
        break;
    case 'R' :      //move right
        // brake();
        rightTurn();
        break;

    case 'L' :      //move left
       // brake();
        leftTurn();
        break;
    
    
    case 'B' :      //move back
       // brake();
     cellBack();
        break;
    }
    
}

void setup()
{
    tofSetup();
    motorDiver();
    motorInterrupt();
    attachInterrupt(digitalPinToInterrupt(PB12), countLeftOut1, RISING);
    attachInterrupt(digitalPinToInterrupt(PB13), countLeftOut1, RISING);
    attachInterrupt(digitalPinToInterrupt(PB14), countRightOut1, RISING);
    attachInterrupt(digitalPinToInterrupt(PB15), countRightOut1, RISING);
    Serial2.begin(9600);
    Serial2.print("READY");
}

void loop()
{/*Serial2.print("READY");

    digitalWrite(led, LOW); `
    digitalWrite(led, HIGH);
    */delay(1000);
    while(1)
    {

      wallFollow();
 
   mazeSolve();


 //rightAboutTurn();+++++++++++
  
 // leftPid();
//wallPid();
    // rightPid();
    //8 tofPid();
    
    
     //tofCell();
   // leftPwm = leftBase - correction;
   // rightPwm = rightBase + correction;

     //  printTof();

    }
}
