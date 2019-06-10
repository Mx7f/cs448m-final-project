#include "linkageXY.h"
#include "vec2.h"
#include "braille.h"

#define X_BOUNDS 75

vec2f ORIGIN = {0,60};

vec2f currentCharPos = ORIGIN;

vec2f CHAR_SPACING = {9,10};

vec2f DOT_SPACING  = {4.5,5};

vec2f CORNER_SPACING = {DOT_SPACING.x+CHAR_SPACING.x, DOT_SPACING.y*2+CHAR_SPACING.y};

int complete = 0;

void setup() {
  absoluteMode = 1;
  // For debugging and GCode communication
  Serial.begin(9600);
  // lower arm, upper arm, pen up
  attachServoPins(4,6,2);
  goToOrigin();
  Serial.println("Setup Complete");

  char message[] = "Hello World";
  int charCount = strlen(message);
  for (int i = 0; i < charCount; ++i) {
    doBraille(message[i]);
  }
}

// If embossing, use inverted
void drawBrailleChar(char c, vec2f upperLeft, vec2f spacing, int inverted) {
  Serial.print("drawBrailleChar(");
  Serial.print((int)c);
  Serial.println(")");
  unsigned char encoding = brailleDots[(int)c];
  for (int x = 0; x < 2; ++x) {
    for (int y = 0; y < 3; ++y) {
      if (isBitSet(encoding, x*3+y)) {
        int xOff = (inverted != 0) ? x : (1-x);
        float xf = upperLeft.x + (xOff*spacing.x);
        float yf = upperLeft.y - (y*spacing.y);
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
  Serial.print("doBraille(");
  Serial.print((int)c);
  Serial.println(")");
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
      drawBrailleChar(brailleIdx, currentCharPos, DOT_SPACING, 0);
      advanceSpacing();
    }
  }
}


void loop() {
                         
}
