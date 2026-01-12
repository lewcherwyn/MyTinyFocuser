#ifndef MOTORCONTROL_H
#define MOTORCONTROL_H

#include <Arduino.h>

// --------------------- Motor Control Code ---------------------

// Pin definitions
extern int pin1; 
extern int pin2; 
extern int pin3; 
extern int pin4;
extern int delaytime; // delay in ms

// EEPROM相关函数声明
void loadAllSettingsFromEEPROM();
void saveAllSettingsToEEPROM();
void loadPositionFromEEPROM();    // 向后兼容
void savePositionToEEPROM();      // 向后兼容

// MovingType
enum MovingType {
  ABS, // Absolute movement
  REL  // Relative movement
};
extern MovingType movingType;

// MotorDirection
enum MotorDirection {
  FORWARD,
  BACKWARD
};
extern MotorDirection currentDirection;  // 原 currentDirectionn 改为 currentDirection

// MotorMode
enum MotorMode {
  MODE_DS8, // Dual 4-step mode
  MODE_S4 // Half-step 8-step mode
};
extern MotorMode currentMode;

// MotorState
enum MotorState { 
  IDLE,
  RUNNING 
};
extern MotorState motorState;

enum TempComp { 
  OFF,
  ON 
};
extern TempComp tempComp;

// Current step position, initialized to 5000
extern long currentPosition;
extern long maxstep; 
extern long maxIncrement; 
extern int multiplier;

// Halt command, used to detect a halt command during motor movement
extern volatile bool haltRequested;

// motor control functions
void rotateStepsForward(int steps, int mode);
void rotateStepsReverse(int steps, int mode);

// -------------------Dual 4-Step Mode-------------------//
// Forward full cycle (4 steps)
int S4Pai();
// Remaining steps (1-3 steps)
int partialS4Pai(int n);
// Reverse full cycle (4 steps)
int S4PaiReverse();
// Reverse Remaining steps (1–3 steps)
int partialS4PaiReverse(int n);

// -------------------Half-Step 8-Step Mode-------------------//
// Forward full cycle (8 steps)
int DS8Pai();
// Remaining steps (1-7 steps)
int partialDS8Pai(int n);
// Reverse full cycle (9 steps)
int DS8PaiReverse();
// Remaining steps (1-7 steps)
int partialDS8PaiReverse(int n);


// // Safe delay function:
// Splits totalDelay into 1 ms delays and polls serial input for a halt command
bool safeDelay(int totalDelay);

#endif
