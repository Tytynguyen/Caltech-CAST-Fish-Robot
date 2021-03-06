/*
 * Tyler Nguyen 2020
 * tylernguyen@caltech.edu
 * Communicates with MATLAB over Serial in order to receive trajectory coordinates on Arduino.
 * Run ArduinoCode first, then run MATLAB. Will run until it has received the entire trajectory.
 */

#include <string.h>
#include <CONTROL.h> //import control library : library homemade that contains all the functions to control the different servos

//initialization of the different angles
float pwmRotation = 90;
float upDownAngle = 90;
float leftRightAngle = 90;
float timestep = 0;

int numPos = 0; //num position vectors to expect from MATLAB
const byte numChars = 16; //Max num chars to expect when reading from serial for each value
char recvChars[4][numChars]; //Stores the current vector values. (timestep,phi,theta,psi)

boolean running = true;
boolean newData = false;

CONTROL control(5, 6, 9, 10, 11, 1);/*constructor of the object Control -> the 5 first numbers are for the number of 
the pins 5,6 for right,left, 9,10 for up, down and 11 for continuous servo*/

void setup() {
    Serial.begin(250000);
    Serial.setTimeout(100);

    while(Serial.available() == 0);

    //Set number of trajectory positions to expect from MATLAB
    numPos = Serial.readStringUntil('\n').toInt();
    
    Serial.print("Num vectors that ARDUINO expects: ");
    Serial.println(numPos);

    initControl();
}

void loop() {
  if(running){
    recvData();

    if (newData){
      parseData();
      newData = false;
      returnData();
      runData();
    }
  }
}

void initControl(){
  control.init();
//  control.initLeftRight();
//  control.initUpDown();
//  control.initRotServo(); 
}

void runData(){

  float upDownMS = map(upDownAngle, 0, 180, 1000, 2000);
  float leftRightMS = map(leftRightAngle, 0, 180, 1000, 2000);
  control.controlUpDown(upDownMS);
  control.controlLeftRight(leftRightMS);
}

void recvData(){
  static boolean recvInProgress = false;
  char startMarker = '<';
  char delimiter = ',';
  char endMarker = '>';

  byte curVar;
  byte curChar;
  
  char received; //Current character received over serial

  while (Serial.available() > 0 && newData == false){
    received = Serial.read();

    if (recvInProgress == true){
      if (received == endMarker){ //End of 3-value pair
        newData = true;
        recvInProgress = false;
        return;
      } else
      if (received == delimiter){ //End of a singular value in vector
        recvChars[curVar][curChar] = '\0'; //Terminate string
        curVar++;
        curChar = 0;
        continue;
      } else { //Add received character to the end of the current angle.
        recvChars[curVar][curChar] = received;
        curChar++;
      }
    }
    else if (received == startMarker){ //Start of 3-value vector
      recvInProgress = true;

    // Clear received characters
      recvChars[0][0] = '\0';
      recvChars[0][1] = '\0';
      recvChars[0][2] = '\0';
      recvChars[0][3] = '\0';

      curVar = 0;
      curChar = 0;
    }
  }
}

void parseData(){
    // Convert character arrays to floats
    timestep = atof(recvChars[0]);
    pwmRotation = atof(recvChars[1]);
    upDownAngle = atof(recvChars[2]);
    leftRightAngle = atof(recvChars[3]);
}

void returnData(){
    //send received vector back to MATLAB
    Serial.print("Timestep:");
    Serial.print(timestep);
    Serial.print("PWM:");
    Serial.print(pwmRotation);
    Serial.print("|UD:");
    Serial.print(upDownAngle);
    Serial.print("|LR:");
    Serial.println(leftRightAngle);
}
