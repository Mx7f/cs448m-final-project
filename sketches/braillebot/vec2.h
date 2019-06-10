#pragma once

struct vec2i {
  int x, y;
};
struct vec2f {
  float x, y;
};


vec2i sub(vec2i p1, vec2i p0) {
  return {p1.x - p0.x, p1.y - p0.y};
}

vec2f subf(vec2f p1, vec2f p0) {
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
