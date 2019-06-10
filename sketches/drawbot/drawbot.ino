#include "linkageXY.h"
#include "vec2.h"
#include "gcodeCommand.h"

int commsOpen = 0;
int statusOk = 1;

void reset() {
  // We're totally grbl guys
  Serial.print("\r\nGrbl 0.8c ['$' for help]\r\n"); // baud rate for lower than v1 is 9600
  // Go to default position 
  goToOrigin();
  commandLen = 0;
  prepareForCommand();
  delay(100);
}


void setup() {
  absoluteMode = 1;
  // For debugging and GCode communication
  Serial.begin(9600);
  // lower arm, upper arm, pen up
  attachServoPins(4,6,2);
  goToOrigin();
  reset();
  Serial.println("Setup Complete");
}

void setFeedrate(float rate) {
  feedrate = rate; // Not that we do anything with it.
}


// Report the current position state to the hose
void reportState() {
  printsf("X",currentPos.x);
  printsf("Y",currentPos.y);
  Serial.println(absoluteMode ? "ABS" : "REL");
  Serial.println(penUp ? "PEN-UP" : "PEN-DOWN");
}

void help() {
  Serial.println("Help? Nah, this isn't grbl. Read the code!\r\n");
}

void invalidCommand() {
  Serial.print("Invalid Command: ");
  Serial.print(commandBuffer);
  Serial.print("\r\n");
}

#define UNDEFINED_Z 123456789.0
// Read the input buffer and find any recognized commands.  One G or M command per line.
// Only handles G0,1,4,20,21,90,91 and M100,114
void processCommand() {
  int invalid = -1;
  if (commandBuffer[0] == '$') {
    statusOk = 0;
    //if (commandLen==1) {
    help();
    //}
  } else {
    statusOk = 1;
    int cmd = parseNumber('G',invalid);
    switch(cmd) {
      case  0:
      case  1: { // line
        setFeedrate(parseNumber('F',feedrate));
        float x = parseNumber('X', (absoluteMode ? currentPos.x : 0)) + (absoluteMode ? 0 : currentPos.x);
        float y = parseNumber('Y', (absoluteMode ? currentPos.y : 0)) + (absoluteMode ? 0 : currentPos.y);
        float z = parseNumber('Z', UNDEFINED_Z);
        //printsf("X",x);
        //printsf("Y",y);
        //printsf("Z",z);
        if (z != UNDEFINED_Z) { // If there's a Z, set the pen up
          setPenUp(z >= 0);
        }
        // Will do nothing if positions are the same
        lineTo({x,y});
        break;
      }
      case 4: doALittleDance(); break;
      case 20: Serial.println("Imperialism is bad!"); break;
      case 21: break; // Set to mm, which is what we always use
      case 90: absoluteMode=1;  break;  // absolute mode
      case 91: absoluteMode=0;  break;  // relative mode
      default: 
        if (cmd != invalid) invalidCommand();
        break;
    }
    if (cmd == invalid) {
      cmd = parseNumber('M',invalid);
      switch(cmd) {
      case 18:   break;
      case 100:  help();  break;
      case 114:  reportState();  break;
      default:  
        invalidCommand();
        break;
      }
    }
  }
}

void signalReady() {
  Serial.print(F("ok\r\n")); // UGS ready signal (with grbl driver)
}

void prepareForCommand() {
  //Serial.println(commandBuffer);
  //Serial.println(commandLen);
  if (statusOk) { 
    signalReady();
  } else {
    Serial.print(F("\r\n"));
  }
  commandLen=0;
}

// Just for funsies
void doALittleDance() {
  int penUpPrev = penUp;
  vec2f oldPos = currentPos;
  setPenUp(0);
  lineTo({35,-5});
  for (int i = 0; i < 5; ++i) {
    lineTo({45,-5});
    lineTo({25,-5});
  }

  // Four corners
  lineTo({0,0});
  lineTo({0,75});
  lineTo({75,0});
  lineTo({75,75});
  
  vec2f C = {35,10};
  float radius = 15;
  for (float theta = -0.5*3.14159; theta < 5.5*3.14159; theta += 0.05) {
    float dx = cos(theta)*radius;
    float dy = sin(theta)*radius;
    lineTo({C.x+dx,C.y+dy});
  }
  
  for (int i = 0; i < 3; ++i) {
    setPenUp(0);
    setPenUp(1);
  }
  lineTo(oldPos);
  setPenUp(penUpPrev);

}

void loop() {
  //Serial.println("loop");
  // send data only when you receive data:
  if (Serial.available() > 0) {
    if (commsOpen == 0) {
      Serial.println("Hi");
      reset();
      commsOpen = 1;
    }
    char c = Serial.read(); //Reading a character at a time
    //Serial.print(c);
    // If there's space, append the incoming character
    if(commandLen < MAX_COMMAND_LEN && c != '?') { // Saw lots of '?' in the input for some reason.
      commandBuffer[commandLen] = c;
      commandLen++;
    }
    // if we got a return character (\n) the message is done.
    if(c=='\n') {
      // Null terminate
      commandBuffer[commandLen]=0;
      processCommand();
      prepareForCommand();
    }
  }                               
}
