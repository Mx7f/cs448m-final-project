#pragma once
#include <Servo.h>
#include "vec2.h"

// Constants, would be good to move out
#define DEGREES_PER_RADIAN 57.295779513

// Kinematic Constants from https://github.com/bdring/DrawBot_Badge/blob/master/DrawBot_Badge/servo.h
// work area
#define MIN_WORK_X -30.0 // outside work area, but useful for calibration
#define MAX_WORK_X 75.0
#define MIN_WORK_Y 0.0  
#define MAX_WORK_Y 75.0
// arm lengths
#define LEN_PEN_FOREARM   55.0
#define L_PEN_UPPERARM    30.0
#define LEN_LOWER_CRANK   50.0
#define LEN_UPPER_CRANK   30.0
#define LEN_MID_LINK      60.0
// static positions
#define SERVO_A_X 24.0
#define SERVO_A_Y 91.1
#define SERVO_B_X SERVO_A_X // both have same x position
#define SERVO_B_Y 103.1

// Servos
Servo servoA;  
Servo servoB;
Servo servoZ;

vec2f currentPos;
int penUp;
int absoluteMode;
float feedrate;

// TODO: servo writeMicroseconds for finer control.
vec2i currentServoPos;

void setServoPos(vec2i pos) {
  currentServoPos = pos;
  servoA.write(pos.x);
  servoB.write(pos.y);
}

void setPenUp(int up) {
  if (up != penUp) {
    penUp = up;
    servoZ.write(up ? 90 : 0);
    delay(1000);
  }
}

void setDefault() {
  vec2i pos = {120,120};
  setServoPos(pos);
  setPenUp(1);
}


void move(vec2i pos) {
  int distDelay = max(10,(int)Linf_dist(currentServoPos, pos)*5);
  setServoPos(pos);
  delay(distDelay);
}

void dot() {
  setPenUp(0);
  setPenUp(1);
}

void plot(vec2i pos) {
  move(pos);
  dot();
}


// Adopted from https://github.com/bdring/DrawBot_Badge/blob/master/DrawBot_Badge/servo.cpp#L337
// Will probably modify
int circle_circle_intersection(vec2f c0, float r0,  // x,y and radius 
                               vec2f c1, float r1,
                               vec2f* i0, // intersection
                               vec2f* i1) {
  double a, dx, dy, d, h, rx, ry;
  double x2, y2;

  /* dx and dy are the vertical and horizontal distances between
   * the circle centers.
   */
  dx = c1.x - c0.x;
  dy = c1.y - c0.y;

  /* Determine the straight-line distance between the centers. */
  d = hypot(dx,dy); // Suggested by Keith Briggs

  /* Check for solvability. */
  if (d > (r0 + r1) || d < fabs(r0 - r1)) {
    /* No solution. Circles either do not intersect or are contained in each other */
    return 0;
  }
  
  // 'point 2' is the point where the line through the circle
  // intersection points crosses the line between the circle centers.  
  

  /* Determine the distance from point 0 to point 2. */
  a = ((r0*r0) - (r1*r1) + (d*d)) / (2.0 * d) ;

  /* Determine the coordinates of point 2. */
  x2 = c0.x + (dx * a/d);
  y2 = c0.y + (dy * a/d);

  // Determine the distance from point 2 to either of the
  // intersection points.
  h = sqrt((r0*r0) - (a*a));

  /* Now determine the offsets of the intersection points from
   * point 2.
   */
  rx = -dy * (h/d);
  ry = dx * (h/d);

  /* Determine the absolute intersection points. */
  *i0 = {x2 + rx,y2 + ry};
  *i1 = {x2 - rx,y2 - ry};

  return 1;
}

// Modified from https://github.com/bdring/DrawBot_Badge/blob/master/DrawBot_Badge/servo.cpp#L213
void computeServoAngles(vec2f pos) {
  float angle_servo_a, angle_servo_b; // the angle of servo arms relative to machine
  vec2f isect0, isect1; // two possible intersection points 
  vec2f pin1, pin2, pin3;
  
  // ============== find the location of pin1 ========================
  if (circle_circle_intersection(pos, LEN_PEN_FOREARM,  // pen location and arm length 
                               {SERVO_A_X, SERVO_A_Y}, LEN_LOWER_CRANK, // servo a location and arm length
                               &isect0, &isect1) == 0) {
    Serial.println("pin1 calc fail");  
    return;
  }                               
  
  // Right-most link is correct
  pin1 = (isect0.x > isect1.x) ? isect0 : isect1;
  
  // ============== find the angle of Servo A =======================
  angle_servo_a = atan2(pin1.y - SERVO_A_Y, pin1.x - SERVO_A_X);
  
  // =============== find location of pin2 ========================
  float R = ((LEN_PEN_FOREARM + L_PEN_UPPERARM)/LEN_PEN_FOREARM);
  pin2.x = ((pin1.x-pos.x)*R)+pos.x;
  pin2.y = ((pin1.y-pos.y)*R)+pos.y;
    
  // ============== find the location of pin3 ========================
  if (circle_circle_intersection(pin2, LEN_MID_LINK,  // pen location and arm length 
                               {SERVO_B_X, SERVO_B_Y}, LEN_UPPER_CRANK, // servo a location and arm length
                               &isect0, &isect1) == 0) {
    Serial.println("pin3 calc fail");
    return;
  } 

  // Larger Y is correct
  pin3 = (isect0.y > isect1.y) ? isect0 : isect1;
    
  // ============== find the angle of Servo B =======================
  angle_servo_b = atan2(pin3.y - SERVO_B_Y, pin3.x - SERVO_B_X);
  
  float a_degrees = angle_servo_a*DEGREES_PER_RADIAN;
  float b_degrees = angle_servo_b*DEGREES_PER_RADIAN;
  // Lots of diagramming to get these constants
  int a_calibrated = a_degrees + 120;
  int b_calibrated = b_degrees + 30;
  
  move({a_calibrated,b_calibrated});
  /*Serial.print("A: ");
  Serial.println(a_degrees);
  Serial.print("B: ");
  Serial.println(b_degrees);*/
  
}

#define LINE_STEP 1.0

// Adapted from https://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm
void line(vec2f p0, vec2f p1) {
  vec2f delta = subf(p1,p0);
  if (delta.x == 0.0f) {// Handle (vertical line)
    float x = p0.x;
    float yInc = (delta.y>0) ? LINE_STEP : -LINE_STEP;
    for (float y = p0.y; y != (p1.y + yInc); y += yInc) {
      computeServoAngles({x,y});
    }
  } else {
    float deltaerr = abs((float)delta.y / (float)delta.x);    
    float sgnDY = (float)((delta.y > 0) - (delta.y < 0));
    float error = 0.0f; // No error at start
    float y = p0.y;
    float xInc = (delta.x>0) ? LINE_STEP : -LINE_STEP;
    for (float x = p0.x; x != (p1.x + xInc); x += xInc) {
      computeServoAngles({x,y});
      error = error + deltaerr;
      if (error >= 0.5f) {
        y = y + sgnDY;
        error -= 1.0f;
      }
    }   
  }
}

// Adapted from https://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm
void dotLine(vec2i p0, vec2i p1) {
  vec2i delta = sub(p1,p0);
  if (delta.x == 0) {// Handle (vertical line)
    int x = p0.x;
    for (int y = p0.y; y <= p1.y; ++y) {
      plot({x,y});
    }
  } else {
    float deltaerr = abs((float)delta.y / (float)delta.x);    
    int sgnDY = (delta.y > 0) - (delta.y < 0);
    float error = 0.0f; // No error at start
    int y = p0.y;
    int xInc = (delta.x>0) ? 1 : -1;
    for (int x = p0.x; x != (p1.x + xInc); x += xInc) {
      plot({x,y});//plot({x,y});
      error = error + deltaerr;
      if (error >= 0.5f) {
        y = y + sgnDY;
        error -= 1.0f;
      }
    }   
  }
}

void recenter() {
  move({60, 80});
}

void lineTo(vec2f p) {
  line(currentPos,p);
  currentPos = p;
}

void attachServoPins(int aPin, int bPin, int zPin) {
  servoA.attach(aPin); // lower arm
  servoB.attach(bPin); // upper arm
  servoZ.attach(zPin); // underneath
}
