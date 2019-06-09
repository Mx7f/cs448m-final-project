#include "linkageXY.h"
#include "vec2.h"
#include "gcodeCommand.h"

void setup() {
  absoluteMode = 1;

  // For debugging and GCode communication
  Serial.begin(9600);
  // lower arm, upper arm, pen up
  attachServoPins(4,6,2);
  
  // Go to default position 
  setDefault();
  delay(1000); 
}

void setFeedrate(float rate) {
  feedrate = rate;
  //Serial.println("Note: Feedrate is ignored!");
}

// Prints a [s]tring and then a [f]loat
void printsf(const char *code,float val) {
  Serial.print(code);
  Serial.println(val);
}


// Report the current position state to the hose
void reportState() {
  printsf("X",currentPos.x);
  printsf("Y",currentPos.y);
  Serial.println(absoluteMode ? "ABS" : "REL");
  Serial.println(penUp ? "PEN-UP" : "PEN-DOWN");
}

void help() {
  Serial.println("Read the code!");
}

#define UNDEFINED_Z 123456789.0
// Read the input buffer and find any recognized commands.  One G or M command per line.
// Only handles G0,1,4,20,21,90,91 and M100,114
void processCommand() {
  int invalid = -1;
  int cmd = parseNumber('G',invalid);
  switch(cmd) {
    case  0:
    case  1: { // line
      setFeedrate(parseNumber('F',feedrate));
      float x = parseNumber('X',(absoluteMode ? currentPos.x : 0)) + (absoluteMode ? 0 : currentPos.x);
      float y = parseNumber('Y',(absoluteMode ? currentPos.y : 0)) + (absoluteMode ? 0 : currentPos.y);
      float z = parseNumber('Z',UNDEFINED_Z);
      if (z != UNDEFINED_Z) { // If there's a Z, set the pen up
        setPenUp(z >= 0);
      }
      // Will do nothing if positions are the same
      lineTo({x,y});
      break;
    }
    case 4: // TODO parse and pause break;
    case 20: Serial.println("Imperialism is bad!"); break;
    case 21: break; // Set to mm, which is what we always use
    case 90: absoluteMode=1;  break;  // absolute mode
    case 91: absoluteMode=0;  break;  // relative mode
    default:  
      break;
  }
  if (cmd != invalid) {
    cmd = parseNumber('M',invalid);
    switch(cmd) {
    case 18:   break;
    case 100:  help();  break;
    case 114:  reportState();  break;
    default:  
      Serial.print("Invalid Command");
      Serial.println(commandBuffer);
      break;
    }
  }
}

void loop() {

  setCommand("G1  Z10");
  processCommand();

  setCommand("G1 X0 Y0");
  processCommand();
  setCommand("G1  Z-10");
   
  //setCommand("G1 X0 Y75");
  processCommand();
  for (float y = 0; y < 75; y += 5) {
    lineTo({0,y});
    lineTo({75,y});
    lineTo({0,y});
  }
  setCommand("G1 X0 Y70");
  processCommand();
  setCommand("G1 X70 Y70");
  processCommand();
  setCommand("G1 X70 Y0");
  processCommand();
  setCommand("G1 X0 Y0");
  processCommand();
  
  delay(1000);
  //setCommand("G1 X70 Y70 Z10");
  //processCommand();
  /*
  // Draw horizontal lines
  for (float y = 0; y <= 75; y += 5) {
    for (float x = 0; x <= 75; ++x) {
      computeServoAngles({x,y});
    }
  }
  */
                                        
}
