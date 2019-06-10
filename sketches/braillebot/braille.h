#pragma once
// 3 0
// 4 1
// 5 2
unsigned char brailleDots[] = { //Using octal
  010,
  030,
  011,
  013,
  012,
  031,
  033,
  032,
  021,
  023,
  050,
  070,
  051,
  053,
  052,
  071,
  073,
  072,
  061,
  063,
  054,
  074,
  027,
  055,
  057,
  056
};

int isBitSet(unsigned char set, int bt) {
  return ((set >> bt) & 1) == 1;
}
