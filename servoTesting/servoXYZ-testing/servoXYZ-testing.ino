#include <Servo.h>

// Servo labeling
Servo servoX;  
Servo servoY;
Servo servoZ;

struct ivec2 {
  int x, y;
};

// servo writeMicroseconds for finer control.

int serPos = 0;

ivec2 currentPos;

void setPos(ivec2 pos) {
  currentPos = pos;
  servoX.write(pos.x);
  servoY.write(pos.y);
}

void setDefault() {
  ivec2 pos = {0,0};
  setPos({120,120});
  servoZ.write(90);
}

void setup() {
  servoX.attach(6); // upper arm
  servoY.attach(4); // lower arm
  servoZ.attach(2); // underneath
  // Set 
  //servoZ.write(0);
  //servoX.write(90);
  //servoY.write(90);
  setDefault();
  //delay(1000); 
}

ivec2 sub(ivec2 p1, ivec2 p0) {
  return {p1.x - p0.x, p1.y - p0.y};
}

float dist(ivec2 p0, ivec2 p1) {
  ivec2 diff = sub(p1,p0);
  return sqrt(diff.x*diff.x+diff.y*diff.y);
}


float Linf_dist(ivec2 p0, ivec2 p1) {
  ivec2 diff = sub(p1,p0);
  return max(abs(diff.x),abs(diff.y));
}




void plot(ivec2 pos) {
  int distDelay = min(10,(int)Linf_dist(currentPos, pos)*5);
  setPos(pos);
  delay(distDelay);
  servoZ.write(0);
  delay(100);
  servoZ.write(90);
  delay(100);
}

void drawDottedLine(ivec2 p0, ivec2 p1) {
  ivec2 delta = sub(p1,p0);
  if (delta.x == 0) {
    int x = p0.x;
    for (int y = p0.x; y <= p0.y; ++y) {
      plot({x,y});
    }
  } else {
    float deltaerr = abs((float)delta.y / (float)delta.x);    // Assume deltax != 0 (line is not vertical),
             // note that this division needs to be done in a way that preserves the fractional part
    int sgnDY = (delta.y > 0) - (delta.y < 0);
    float error = 0.0f; // No error at start
    int y = p0.y;
    for (int x = p0.x; x <= p1.x; ++x) {
      plot({x,y});
      error = error + deltaerr;
      if (error >= 0.5f) {
        y = y + sgnDY;
        error = error - 1.0f;
      }
    }   
  }
}

void loop() {
  // Counter Clockwise from "origin" at top left
  ivec2 corners[] = {{0,60}, {65,50}, {140,80}, {160,160}};

  for (int i = 0; i < 4; ++i) {
    //drawDottedLine(corners[i], corners[(i+1)%4]);
  }

  for (int i = 0; i < 4; ++i) {
    drawDottedLine(corners[i], corners[(i+2)%4]);
  }
                                        
}
