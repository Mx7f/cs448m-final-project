#include <Servo.h>

// Servo labeling
Servo servoX;  
Servo servoY;
Servo servoZ;

int serPos = 0;

void setup() {
  servoX.attach(4); // upper arm
  servoY.attach(6); // lower arm
  servoZ.attach(2); // underneath, should not be more than 90?
}

void loop() {
  
  servoX.write(10);                  
  delay(100);                           
 
  servoY.write(170);                  
  delay(1000); 
  
  servoZ.write(45);
  delay(100);                         
}
