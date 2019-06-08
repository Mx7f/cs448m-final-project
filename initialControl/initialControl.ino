#include <Servo.h>

// Servo labeling
Servo servoA;  
Servo servoB;
Servo servoZ;

struct vec2i {
  int x, y;
};
struct vec2f {
  float x, y;
};

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


// TODO: servo writeMicroseconds for finer control.
vec2i currentPos;

void setPos(vec2i pos) {
  currentPos = pos;
  servoA.write(pos.x);
  servoB.write(pos.y);
}

void setDefault() {
  vec2i pos = {0,0};
  setPos({120,120});
  servoZ.write(0);
}

void setup() {
  // For debugging and/or GCode communication
  Serial.begin(9600);
  servoA.attach(4); // lower arm
  servoB.attach(6); // upper arm
  servoZ.attach(2); // underneath
  
  // Go to default position 
  setDefault();
  delay(1000); 
}

vec2i sub(vec2i p1, vec2i p0) {
  return {p1.x - p0.x, p1.y - p0.y};
}


float dist(vec2i p0, vec2i p1) {
  vec2i diff = sub(p1,p0);
  return sqrt(diff.x*diff.x+diff.y*diff.y);
}


float Linf_dist(vec2i p0, vec2i p1) {
  vec2i diff = sub(p1,p0);
  return max(abs(diff.x),abs(diff.y));
}



void move(vec2i pos) {
  int distDelay = max(10,(int)Linf_dist(currentPos, pos)*5);
  setPos(pos);
  delay(distDelay);
}

void dot() {
  servoZ.write(0);
  delay(100);
  servoZ.write(90);
  delay(100);
}

void plot(vec2i pos) {
  move(pos);
  dot();
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


// Modified from https://github.com/bdring/DrawBot_Badge/blob/master/DrawBot_Badge/servo.cpp#L213
void compute_servo_angles(vec2f pos) {
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


void loop() {
  delay(100);

  // Draw horizontal lines
  for (float y = 0; y <= 75; y += 5) {
    for (float x = 0; x <= 75; ++x) {
      compute_servo_angles({x,y});
    }
  }

  
  /*// Counter Clockwise from "origin" at top left
  vec2i corners[] = {{60,0}, {50,65}, {80,140}, {160,160}};

  for (int i = 0; i < 4; ++i) {
    //drawDottedLine(corners[i], corners[(i+1)%4]);
  }

  for (int i = 0; i < 4; ++i) {
    recenter();
    drawDottedLine(corners[i], corners[(i+2)%4]);
  }*/
                                        
}
