#include "linkageXY.h"
#include "vec2.h"

void setup() {
  // For debugging and GCode communication
  Serial.begin(9600);
  // lower arm, upper arm, pen up
  attachServoPins(4,6,2);
  setDefault();
}

void loop() {
  setPenUp(0);
  setPenUp(1);                      
}
