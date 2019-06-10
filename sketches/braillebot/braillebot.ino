#include "linkageXY.h"
#include "vec2.h"
#include "braille.h"

#define X_BOUNDS 70

vec2f ORIGIN = {0,60};

vec2f currentCharPos = ORIGIN;

vec2f CHAR_SPACING = {9,10};

vec2f DOT_SPACING  = {4.5,5};

vec2f CORNER_SPACING = {DOT_SPACING.x+CHAR_SPACING.x, DOT_SPACING.y*2+CHAR_SPACING.y};

int complete = 0;

int mirror = 0;

void setup() {
  absoluteMode = 1;
  // For debugging and GCode communication
  Serial.begin(9600);
  // lower arm, upper arm, pen up
  attachServoPins(4,6,2);
  setPenUp(1);
  computeServoAngles(ORIGIN);
  currentPos = ORIGIN;
  Serial.println("Setup Complete");
}

// If embossing, use inverted
void drawBrailleChar(char c, vec2f upperLeft, vec2f spacing) {
  Serial.print("drawBrailleChar(");
  Serial.print((int)c);
  Serial.println(")");
  unsigned char encoding = brailleDots[(int)c];
  for (int x = 0; x < 2; ++x) {
    for (int y = 0; y < 3; ++y) {
      if (isBitSet(encoding, x*3+y)) {
        int xOff = 1-x;
        float xf = upperLeft.x + (xOff*spacing.x);
        float yf = upperLeft.y - (y*spacing.y);
        if (mirror) {
          xf = X_BOUNDS-xf;
        }
        makeDot({xf,yf});
      }
    }
  }
}

void advanceRow() {
  currentCharPos.y -= CORNER_SPACING.y;
  currentCharPos.x = ORIGIN.x;
  float bottomY = currentCharPos.y - CORNER_SPACING.y + CHAR_SPACING.y;
  if (bottomY < 0) {
    complete = 1;
  }
}

void advanceSpacing() {
  currentCharPos.x += CORNER_SPACING.x;
  if (currentCharPos.x + DOT_SPACING.x > X_BOUNDS) {
    advanceRow();
  }
}

char LOWER_A = 'a';
char UPPER_A = 'A';

void doBraille(char c) {
  if (!complete) {
    char brailleIdx;
    int doit = 0;
    if (c == ' ') {
      advanceRow();
    } else if (c >= UPPER_A && c < UPPER_A+26) { // Uppercase
      brailleIdx = c - UPPER_A;
      doit = 1;
    } else if (c >= LOWER_A && c < LOWER_A+26) {
      brailleIdx = c - LOWER_A;
      doit = 1;
    }
    if (doit) {
      drawBrailleChar(brailleIdx, currentCharPos, DOT_SPACING);
      advanceSpacing();
    }
  }
}

// Just for funsies
void doALittleDance() {
  int penUpPrev = penUp;
  vec2f oldPos = currentPos;
  setPenUp(1);
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
    fastPenUpFar(0);
    fastPenUpFar(1);
  }
  fastPenUpFar(0);
  setPenUp(1);
  lineTo(oldPos);
  setPenUp(penUpPrev);

}

void loop() {
  if (Serial.available() > 0) {
    char c = Serial.read(); //Reading a character at a time
    if(c == ' ' || (c >= UPPER_A && c < UPPER_A+26) || (LOWER_A && c < LOWER_A+26)) {
      doBraille(c);
    }
    if(c == '$') {
      complete = 0;
      currentCharPos = ORIGIN;
    }
    if(c == '*') { // Reverse mode
      mirror = (mirror == 0);
      Serial.println(mirror ? "EMBOSS" : "DRAW");
    }
    if (c == '!') {
      doALittleDance();
    }
  }            
}
