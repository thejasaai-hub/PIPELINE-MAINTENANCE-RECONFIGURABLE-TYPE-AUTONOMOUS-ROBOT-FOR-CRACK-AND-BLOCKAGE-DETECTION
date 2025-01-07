#include <SoftwareSerial.h>
#include <Wire.h>

SoftwareSerial HC12(0, 1);//tx,rx
#define MOTOR_A1 2
#define MOTOR_A2 3
#define MOTOR_B1 4
#define MOTOR_B2 5

int variable1=0;
int variable2=0;

String input;
int boundLow;
int boundHigh;
const char delimiter = ',';
const int joystick_threshold = 256;


void setup() {
  Serial.begin(9600);
  HC12.begin(9600);
  pinMode(MOTOR_A1, OUTPUT);
  pinMode(MOTOR_A2, OUTPUT);
  pinMode(MOTOR_B1, OUTPUT);
  pinMode(MOTOR_B2, OUTPUT);
}


void loop() {
  if(HC12.available())
  {
  input = HC12.readStringUntil('\n');
  if (input.length() > 0)
      {
        
       
       boundLow = input.indexOf(delimiter);
        variable1 = input.substring(0, boundLow).toInt();
       
        boundHigh = input.indexOf(delimiter, boundLow+1);
        variable2 = input.substring(boundLow+1, boundHigh).toInt();
     

        int leftMotorSpeed = map(variable1,0,1024, -255, 255);
        int rightMotorSpeed = map(variable2 ,0,1024, -255, 255);
 Serial.println(leftMotorSpeed);
 Serial.println(rightMotorSpeed);
  // Control motors
  if (leftMotorSpeed > 100 && rightMotorSpeed < 100)  {
    digitalWrite(MOTOR_A1, HIGH);
    digitalWrite(MOTOR_A2, LOW);
    digitalWrite(MOTOR_B1, HIGH);
    digitalWrite(MOTOR_B2, LOW);
  } else  if (leftMotorSpeed < -100 && rightMotorSpeed > -100){
    digitalWrite(MOTOR_A1, LOW);
    digitalWrite(MOTOR_A2, HIGH);
    digitalWrite(MOTOR_B1, LOW);
    digitalWrite(MOTOR_B2, HIGH);
  }else{
    digitalWrite(MOTOR_A1, LOW);
    digitalWrite(MOTOR_A2, LOW);
    digitalWrite(MOTOR_B1, LOW);
    digitalWrite(MOTOR_B2, LOW);
  }
  if (rightMotorSpeed > 100 && leftMotorSpeed < 100 ) {
    digitalWrite(MOTOR_A1, HIGH);
    digitalWrite(MOTOR_A2, LOW);
    digitalWrite(MOTOR_B1, LOW);
    digitalWrite(MOTOR_B2, HIGH);
  } else if (rightMotorSpeed < -100 && leftMotorSpeed > -100 ) {
    digitalWrite(MOTOR_A1, LOW);
    digitalWrite(MOTOR_A2, HIGH);
    digitalWrite(MOTOR_B1, HIGH);
    digitalWrite(MOTOR_B2, LOW);
  }else{
    digitalWrite(MOTOR_B1, LOW);
    digitalWrite(MOTOR_B2, LOW);
    digitalWrite(MOTOR_B1, LOW);
    digitalWrite(MOTOR_B2, LOW);
  }
//if you change the number of variables, follow instructions in comments above
//duplicating the lines as explained adds 2 variables, then you can duplicate them again, and then change the variable names so they are in the right order
  
delay(10);
      }//after this you can choose what to do with each variable, which will now have the same value as it's value on the transmitter end
  }
}