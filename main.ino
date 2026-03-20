//Code for the RDL Strafer Robot Kit

//Arduino MEGA 2560 Wiring Hookup guide:
//Motor 1(Front Left): 2 
//Motor 2(Back Left): 3
//Motor 3(Back Right): 4
//Motor 4(Front Right): 5
//RoboClaw uses TX1 (pins 18 and 19) for communication

//PS2 Pins referenced down below in code (Lines 17-21)

double armPosition = 0;
double armSpeed = 300;
#include <PS2X_lib.h> //link to connection guide http://www.billporter.info/2010/06/05/playstation-2-controller-arduino-library-v1-0/
#include "RoboClaw.h"
#include <Servo.h>

#define debug true //set to true to get debug data over serial, leave false when using robot for latency

#define RotateSensitivity .6  //rotation sensitivity, the higher the number the faster it rotates, max 1
//PS2 Pins (Black is ground and red is 3V3)
#define PS2_DAT        13  //brown
#define PS2_CMD        11  //orange
#define PS2_SEL        10  //yellow  also known as ATTN - beware:  sometimes controllers flip blue and yellow wire functions.. 
#define PS2_CLK        12  //blue

#define pressures   true  // set to true to help auto-enable required analog controller mode
#define rumble      true  // same

#define address 0x80 //address for the RoboClaw motor controller
#define servoSensitivity 50

#define wheelRadius 2.04724


int analogValue[4]; // variable's for joystick data

RoboClaw roboclaw(&Serial1,10000); //create class for roboclaw, serial number along with timeout


Servo MTR1;  //defining ESC's
Servo MTR2;
Servo MTR3;
Servo MTR4;
Servo ASERVO; //defining Gripper Servo

int DPadV[4];

PS2X ps2x; // create PS2 Controller Class

float time;
int ASERVOVal = 1500;
byte vibrate = 0;
bool motorSaftey = 0; //holds the state of saftey 
bool autonomousState = 0;


void setup() {
  //setup for ESC
  MTR1.attach(2); //FL
  MTR2.attach(3); //BL
  MTR3.attach(4); //BR
  MTR4.attach(5); //FR
  ASERVO.attach(6); //gripper servo

  // configure game pad.   pressures and rumble are set to true - this auto-enables required analog mode.
  ps2x.config_gamepad(PS2_CLK, PS2_CMD, PS2_SEL, PS2_DAT, pressures, rumble);

  if(debug){
    Serial.begin(57600);
  }
  Serial.begin(57600);
  roboclaw.begin(38400);

  delay(500); //delay for allowing serial to initialize

}

// main loop starts here
void loop() {
  ReadPS2(); //Get data from PS2 controller
  armSend(); //Send data to Arm
  motorSend(); //Send data to ESC's
  if(autonomousState){
    autonomous();
  }
  //Serial.println(millis());
  delay(50); //delay for stability  
}

void autonomous(){

  //drive forward


  delay(2000);

  //stop
  MTR1.writeMicroseconds(1500);
  MTR2.writeMicroseconds(1500);
  MTR3.writeMicroseconds(1500);
  MTR4.writeMicroseconds(1500);

  delay(1500);

  //rotate left
  MTR1.writeMicroseconds(1600);
  MTR2.writeMicroseconds(1600);
  MTR3.writeMicroseconds(1600);
  MTR4.writeMicroseconds(1600);

  delay(2350);

  //stop
  MTR1.writeMicroseconds(1500);
  MTR2.writeMicroseconds(1500);
  MTR3.writeMicroseconds(1500);
  MTR4.writeMicroseconds(1500);

  delay(2500);
  //drive forward
  MTR1.writeMicroseconds(1350);
  MTR2.writeMicroseconds(1350);
  MTR3.writeMicroseconds(1650);
  MTR4.writeMicroseconds(1650);

  delay(1500);

  //stop
  MTR1.writeMicroseconds(1500);
  MTR2.writeMicroseconds(1500);
  MTR3.writeMicroseconds(1500);
  MTR4.writeMicroseconds(1500);

  delay(2500);

  ASERVO.writeMicroseconds(1000);


  autonomousState = 0; //variable to define if autonomous should be ran, always set to 0 after you run or it will loop indefinetly. 

}

void moveForward(uint8_t inches){
  MTR1.writeMicroseconds(1350);
  MTR2.writeMicroseconds(1350);
  MTR3.writeMicroseconds(1650);
  MTR4.writeMicroseconds(1650);

  delay(300*inches/12.56);
}
void moveBackward(uint8_t inches){
  MTR1.writeMicroseconds(1650);
  MTR2.writeMicroseconds(1650);
  MTR3.writeMicroseconds(1350);
  MTR4.writeMicroseconds(1350);

  delay(300*inches/12.56);
}
void turnRight(uint8_t angle){
  MTR1.writeMicroseconds(1650);
  MTR2.writeMicroseconds(1650);
  MTR3.writeMicroseconds(1350);
  MTR4.writeMicroseconds(1350);

  delay(100);
}


// read PS2 controller values
void ReadPS2() {
  ps2x.read_gamepad(false, vibrate); //read controller and set large motor to spin at 'vibrate' speed
  analogValue[0] = ps2x.Analog(PSS_LY); //left joystick, y-axis
  analogValue[1] = ps2x.Analog(PSS_LX); //left joystick, x-axis
  analogValue[2] = ps2x.Analog(PSS_RY); //right joystick, y-axis
  analogValue[3] = ps2x.Analog(PSS_RX); //right joystick, x-axis


  //detects if the right bumper has been pressed to toggle safety, can only be toggled once a second
  if(ps2x.Button(PSB_CIRCLE) && millis() - time > 1000) {
    
    if(debug){
      Serial.println("on");
    }

    motorSaftey = !motorSaftey;
      time = millis();
  }

    if(ps2x.Button(PSB_SQUARE) && millis() - time > 1000) {
      autonomousState = 1;
    }



}

void motorSend(){
  //switch for safety mechanism
 switch(motorSaftey){

  case 0:
    if(debug){
      Serial.println("Motors disabled");  //if safety is on, the arduino will not write to the motor controllers
    }
    break;

  case 1: //runs normal code if safety is off
    float XPos = map(analogValue[1], 0, 255, 100, -100);  //convert joystick values to a easier to understand value
    float YPos = map(analogValue[0], 0, 255, 100, -100);
    float TPosa = map(analogValue[3], 0, 255, 100, -100);
    
    float TPos = (TPosa + 100) * (RotateSensitivity + RotateSensitivity) / (100 + 100) -RotateSensitivity; //convert the rotation value to what is needed for the equation
    float atanData = atan2(YPos,XPos) + (.5* PI); //calculate the inverse tangent of x/y (Outputs the rotational value of the joystick in the range of +- 1/2pi)
    atanData = (atanData * -1); //invert calculation

    float MagTemp = sqrt(pow((XPos/100), 2) + pow((YPos/100), 2)); //determine magnitude for speed calculation
    float Mag = (MagTemp - 0) * (1 - 0) / (1.41 - 0) + 0; //convert to the needed range for later calculations

    //calculate the speed for each motor in the range of -1 - +1
    float FL = sin(atanData - (.25 * PI)) * Mag - TPos;
    float BR = sin(atanData - (.25 * PI)) * Mag + TPos;
    float FR = sin(atanData + (.25 * PI)) * Mag - TPos;
    float BL = sin(atanData + (.25 * PI)) * Mag + TPos;

    //convert the previous calculation to the corresponding value in microseconds
    float MTR1Val = (FL + 1) * (1000 - 2000) / (1 + 1) + 2000;
    float MTR2Val = (BL + 1) * (2000 - 1000) / (1 + 1) + 1000;
    float MTR3Val = (BR + 1) * (2000 - 1000) / (1 + 1) + 1000;
    float MTR4Val = (FR + 1) * (1000 - 2000) / (1 + 1) + 2000;

    //Write all motor speeds to the serial monitor for debugging (Comment this out if not needed)

    if(debug){
      Serial.print(FL);
      Serial.print(", ");
      Serial.print(BL);
      Serial.print(", ");
      Serial.print(BR);
      Serial.print(", ");
      Serial.print(FR);
      Serial.print(", ");
      Serial.print(MTR1Val);
      Serial.print(", ");
      Serial.print(MTR2Val);
      Serial.print(", ");
      Serial.print(MTR3Val);
      Serial.print(", ");
      Serial.print(MTR4Val);
      Serial.print(", ");
      Serial.println(armPosition);
    }


    //write the value to the ESC's and Servos
    MTR1.writeMicroseconds(MTR1Val);
    MTR2.writeMicroseconds(MTR2Val);
    MTR3.writeMicroseconds(MTR3Val);
    MTR4.writeMicroseconds(MTR4Val);
    ASERVO.writeMicroseconds(ASERVOVal);
      break;

    }
}

void armSend() {
  if (armPosition > 180 ){ armPosition = 180;}
  if (armPosition > 0){ armPosition = 0;}
  switch(motorSaftey){

    case 1: 
      roboclaw.SpeedAccelDeccelPositionM1(address,0,3000,0,armPosition,0); //sends value to motor controller each loop
      
      if(ps2x.Button(PSB_PAD_UP)) {      //reads up button on dpad and writes to the arm motor
        if (armPosition >= -2500){
          armPosition = armPosition - armSpeed; //decrement arm position
        }
      DPadV[0] = 1;
    }
    else{
      DPadV[0] = 0;
    }

    if(ps2x.Button(PSB_PAD_DOWN)){ //reads down button on dpad and writes to the arm motor
        if(armPosition <= 0 ){
          armPosition = armPosition + armSpeed;  //increment arm position
        }
      DPadV[1] = 1;
    }
    else{
      DPadV[1] = 0;
      }

    if (ps2x.Button(PSB_PAD_LEFT)){
      armPosition = 0;
    }

    if (ps2x.Button(PSB_PAD_RIGHT)){
      armPosition = -2500;
    }

    if(DPadV[0] == 0 && DPadV[1] == 0){  //if no buttons are being pressed keep arm in a steady spot

    }

    if(ps2x.Button(PSB_R1)){  //reads right dpad button and writes to the servo value, opens claw
      if(ASERVOVal <= 2000){
          ASERVOVal = ASERVOVal + servoSensitivity;
      }
      else{
        ASERVOVal = 2000;
      }
      DPadV[2] = 1;
    }
    else{
      DPadV[2] = 0;
    }

    if(ps2x.Button(PSB_L1)){  //reads left dpad button and writes to the servo value, closes claw
      DPadV[3] = 1;
      if(ASERVOVal >= 1000){
        ASERVOVal = ASERVOVal - servoSensitivity;
    }
    else{
      ASERVOVal = 1000;
    } }
    else{
      DPadV[3] = 0;
    }
    break;
    }
}

