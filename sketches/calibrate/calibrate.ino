#include "linkageXY.h"
#include "vec2.h"

void setup() {
  attachServoPins(4,6,2);
  // Go to default position 
  setDefault();
  setPenUp(0);
  delay(1000); 
}

void loop() {
  // Do nothing                          
}
