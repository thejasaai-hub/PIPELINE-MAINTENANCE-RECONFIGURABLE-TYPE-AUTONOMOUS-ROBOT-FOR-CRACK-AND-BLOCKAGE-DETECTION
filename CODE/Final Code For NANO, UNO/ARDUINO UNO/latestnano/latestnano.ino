#include <SoftwareSerial.h>
#include <Wire.h>
SoftwareSerial HC12(11, 10);//tx,rx


int variable1;//variables that will get sent
int variable2;



void setup() {
  HC12.begin(9600);//Sets the hc 12 communication to 9600 baud, which is their factory default
  Serial.begin(9600);
  pinMode(A0, INPUT);
  pinMode(A1, INPUT);
  pinMode(A2, INPUT);
  pinMode(A3, INPUT);
  pinMode(A4, INPUT);
}
void loop() {
  variable1 = analogRead(A0);//assigns the variables that will be sent the value read on the analog pins
  variable2 = analogRead(A1);//you can change this to other data, such as a digital pin's read
  
  //Uncomment the following lines if you want the remote to also send the analog input data to the serial monitor, to check if the data is being read properly
  //If you do, It will look the same as it should on the other end's serial monitor
  Serial.print(variable1);
  Serial.print(",");
  Serial.print(variable2);
  
  Serial.println("");
  HC12.print(variable1);//sends the variables
  HC12.print(",");
  HC12.print(variable2);
 //if you just need to send 2 variables,simply change this value and the following to 0
 // HC12.print(",");
 // HC12.print(variable4);//if you just need to send 3 variables,simply change this value and the next to 0
 // HC12.print(",");
 // HC12.print(variable5);//if you just need to send 4 variables,simply change this value to 0
//if you need 5 or less variables, disregard comments below, but keep the next line of code, it is essential
  HC12.println("");//you can add even numbers of variables, ex add 2, 4, 6 ...(so 3, 5, 7, 9... variables in total) before this line by adding a variable line, then a comma line, another variable line then a second comma line under the 5th variable line but before the println.  
//for the purpose of simplicity, it is easier to add in groups of 2, so that less modification is necessary on the other end
//if you have more variables than you are using, change the variable to 0, as above explained, again for the purpose of simplicity
// This change must be reflected on receiver end: see below
  delay(100);
}
