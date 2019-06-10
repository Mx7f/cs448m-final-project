#pragma once
// Global State
#define MAX_COMMAND_LEN 127
char commandBuffer[MAX_COMMAND_LEN+1];
size_t commandLen;

// Return the float following `code` in the command buffer
// If it doesn't exist; return the default value.
// This is a silly inefficient way to parse a gcode command, but it'll do for now
// Adapted from https://github.com/MarginallyClever/GcodeCNCDemo/blob/master/GcodeCNCDemo2Axis/GcodeCNCDemo2Axis.ino#L179
float parseNumber(char code, float val) {
  long commandEnd = (long)(commandBuffer+commandLen);
  for(char *ptr=commandBuffer; // Start at beginning of buffer
        (long)ptr > 1 && (*ptr) && (long)ptr < commandEnd; 
        ++ptr) {
    if (*ptr==code) { 
      return atof(ptr+1);  // Get float following the code char
    }
  }
  return val;  // end reached, nothing found, return default val.
}

void setCommand(const char* command) {
  strcpy(commandBuffer, command);
  commandLen = strlen(command);
}
