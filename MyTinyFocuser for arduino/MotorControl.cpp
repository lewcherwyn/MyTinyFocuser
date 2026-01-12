#include "MotorControl.h"

// --------------------- Motor Control Code ---------------------

// Pin definitions
int pin1 = 10;
int pin2 = 9;
int pin3 = 8;
int pin4 = 7;
int delaytime = 3; //delay in ms

long maxIncrement = 250;
long currentPosition = 5000;
long maxstep = 10000;
MotorDirection currentDirection = FORWARD;
MotorMode currentMode = MODE_DS8;  // default setting
MotorState motorState = IDLE;
TempComp tempComp = ON;
MovingType movingType = ABS;

volatile bool haltRequested = false;
int local_multiplier = (currentDirection == FORWARD) ? 1 : -1;


static void resetPins() {
  digitalWrite(pin1, LOW);
  digitalWrite(pin2, LOW);
  digitalWrite(pin3, LOW);
  digitalWrite(pin4, LOW);
}


bool safeDelay(int totalDelay) {

  int checkInterval = min(20, max(1, totalDelay / 2));
  int elapsed = 0;
  
  while (elapsed < totalDelay) {
    int sleepTime = min(checkInterval, totalDelay - elapsed);
    delay(sleepTime);
    elapsed += sleepTime;
    
    if (Serial.available() > 0) {
      String input = Serial.readStringUntil('\n');
      input.trim();
      if(input.equalsIgnoreCase("halt")){
        haltRequested = true;
        motorState = IDLE;
        savePositionToEEPROM();  // Immediately save current position
        return false;
      }
      else if(input.equalsIgnoreCase("isMoving")){
        Serial.println(motorState);
      }
      else if(input.equalsIgnoreCase("maxstep")) {
        Serial.println(maxstep);
      } 
      else if(input.equalsIgnoreCase("current:position")){
        Serial.println(currentPosition);
      }
      else if(input.equalsIgnoreCase("Temperature")){
        Serial.println(20);
      }
      else if(input.equalsIgnoreCase("currentState")){
        Serial.println(String(currentPosition) + ";" + String(motorState) + ";" + String(currentMode));
      }
      else if(input.equalsIgnoreCase("deviceState")){
        String output = String(maxstep) + ";" + String(movingType) + ";" + String(currentMode) + ";" + String(currentDirection);
        Serial.println(output);
      }
    }
  }
  return true;
}

//-------------------Dual 4-Step Mode-------------------//
int S4Pai(){
  int stepsCompleted = 0;
  
  digitalWrite(pin1, HIGH); // step1: (1,1,0,0)
  digitalWrite(pin2, HIGH);
  if(!safeDelay(delaytime)) return stepsCompleted;
  stepsCompleted++;
  currentPosition++;  
  
  digitalWrite(pin1, LOW);  // step2: (0,1,1,0)
  digitalWrite(pin3, HIGH);
  if(!safeDelay(delaytime)) return stepsCompleted;
  stepsCompleted++;
  currentPosition++;
  
  digitalWrite(pin2, LOW);  // step3: (0,0,1,1)
  digitalWrite(pin4, HIGH);
  if(!safeDelay(delaytime)) return stepsCompleted;
  stepsCompleted++;
  currentPosition++;
  
  digitalWrite(pin3, LOW);  // step4: (1,0,0,1)
  digitalWrite(pin1, HIGH);
  if(!safeDelay(delaytime)) return stepsCompleted;
  stepsCompleted++;
  currentPosition++;
  
  digitalWrite(pin4, LOW);
  digitalWrite(pin2, HIGH);
  safeDelay(delaytime);
  
  return stepsCompleted;
}

int partialS4Pai(int n) {
  int stepsCompleted = 0;

  if(n >= 1) {
    digitalWrite(pin1, LOW);  // step1: (0,1,1,0)
    digitalWrite(pin3, HIGH);
    if(!safeDelay(delaytime)) return stepsCompleted;
    stepsCompleted++;
    currentPosition++;
  }
  if(n >= 2) {
    digitalWrite(pin2, LOW);  // step2: (0,0,1,1)
    digitalWrite(pin4, HIGH);
    if(!safeDelay(delaytime)) return stepsCompleted;
    stepsCompleted++;
    currentPosition++;
  }
  if(n >= 3) {
    digitalWrite(pin3, LOW);  // step3: (1,0,0,1)
    digitalWrite(pin1, HIGH);
    if(!safeDelay(delaytime)) return stepsCompleted;
    stepsCompleted++;
    currentPosition++;
  }
  return stepsCompleted;
}

// inverse
int S4PaiReverse(){
  int stepsCompleted = 0;
  
  digitalWrite(pin1, HIGH);  // step1: (1,0,0,1)
  digitalWrite(pin4, HIGH);
  if(!safeDelay(delaytime)) return stepsCompleted;
  stepsCompleted++;
  currentPosition--;
  
  digitalWrite(pin1, LOW);   // step2: (0,0,1,1)
  digitalWrite(pin3, HIGH);
  if(!safeDelay(delaytime)) return stepsCompleted;
  stepsCompleted++;
  currentPosition--;
  
  digitalWrite(pin4, LOW);   // step3: (0,1,1,0)
  digitalWrite(pin2, HIGH);
  if(!safeDelay(delaytime)) return stepsCompleted;
  stepsCompleted++;
  currentPosition--;
  
  digitalWrite(pin3, LOW);   // step4: (1,1,0,0)
  digitalWrite(pin1, HIGH);
  if(!safeDelay(delaytime)) return stepsCompleted;
  stepsCompleted++;
  currentPosition--;
  
  digitalWrite(pin2, LOW);
  digitalWrite(pin4, HIGH);
  safeDelay(delaytime);
  
  return stepsCompleted;
}


int partialS4PaiReverse(int n) {
  int stepsCompleted = 0;

  if(n >= 1) {
    digitalWrite(pin1, LOW);  // step1: (0,0,1,1)
    digitalWrite(pin3, HIGH);
    if(!safeDelay(delaytime)) return stepsCompleted;
    stepsCompleted++;
    currentPosition--;
  }
  if(n >= 2) {
    digitalWrite(pin4, LOW);  // step2: (0,1,1,0)
    digitalWrite(pin2, HIGH);
    if(!safeDelay(delaytime)) return stepsCompleted;
    stepsCompleted++;
    currentPosition--;
  }
  if(n >= 3) {
    digitalWrite(pin3, LOW);  // step3: (1,1,0,0)
    digitalWrite(pin1, HIGH);
    if(!safeDelay(delaytime)) return stepsCompleted;
    stepsCompleted++;
    currentPosition--;
  }
  return stepsCompleted;
}

//-------------------Half-Step 8-Step Mode-------------------//
int DS8Pai(){
  int stepsCompleted = 0;
  int local_multiplier = (currentDirection == FORWARD) ? 1 : -1;

  digitalWrite(pin1, HIGH); // step1: (1,0,0,0)
  if(!safeDelay(delaytime)) return stepsCompleted;
  stepsCompleted++;
  currentPosition += local_multiplier;
  
  digitalWrite(pin2, HIGH); // step2: (1,1,0,0)
  if(!safeDelay(delaytime)) return stepsCompleted;
  stepsCompleted++;
  currentPosition += local_multiplier;
  
  digitalWrite(pin1, LOW);  // step3: (0,1,0,0)
  if(!safeDelay(delaytime)) return stepsCompleted;
  stepsCompleted++;
  currentPosition += local_multiplier;
  
  digitalWrite(pin3, HIGH); // step4: (0,1,1,0)
  if(!safeDelay(delaytime)) return stepsCompleted;
  stepsCompleted++;
  currentPosition += local_multiplier;
  
  digitalWrite(pin2, LOW);  // step5: (0,0,1,0)
  if(!safeDelay(delaytime)) return stepsCompleted;
  stepsCompleted++;
  currentPosition += local_multiplier;
  
  digitalWrite(pin4, HIGH); // step6: (0,0,1,1)
  if(!safeDelay(delaytime)) return stepsCompleted;
  stepsCompleted++;
  currentPosition += local_multiplier;
  
  digitalWrite(pin3, LOW);  // step7: (0,0,0,1)
  if(!safeDelay(delaytime)) return stepsCompleted;
  stepsCompleted++;
  currentPosition += local_multiplier;
  
  digitalWrite(pin1, HIGH); // step8: (1,0,0,1)
  if(!safeDelay(delaytime)) return stepsCompleted;
  stepsCompleted++;
  currentPosition += local_multiplier;
  
  digitalWrite(pin4, LOW);
  safeDelay(delaytime);
  
  return stepsCompleted;
}

int partialDS8Pai(int n) {
  int stepsCompleted = 0;
  int local_multiplier = (currentDirection == FORWARD) ? 1 : -1;

  if(n >= 1) {
    digitalWrite(pin2, HIGH);
    if(!safeDelay(delaytime)) return stepsCompleted;
    stepsCompleted++;
    currentPosition += local_multiplier;
  }
  if(n >= 2) {
    digitalWrite(pin1, LOW);
    if(!safeDelay(delaytime)) return stepsCompleted;
    stepsCompleted++;
    currentPosition += local_multiplier;
  }
  if(n >= 3) {
    digitalWrite(pin3, HIGH);
    if(!safeDelay(delaytime)) return stepsCompleted;
    stepsCompleted++;
    currentPosition += local_multiplier;
  }
  if(n >= 4) {
    digitalWrite(pin2, LOW);
    if(!safeDelay(delaytime)) return stepsCompleted;
    stepsCompleted++;
    currentPosition += local_multiplier;
  }
  if(n >= 5) {
    digitalWrite(pin4, HIGH);
    if(!safeDelay(delaytime)) return stepsCompleted;
    stepsCompleted++;
    currentPosition += local_multiplier;
  }
  if(n >= 6) {
    digitalWrite(pin3, LOW);
    if(!safeDelay(delaytime)) return stepsCompleted;
    stepsCompleted++;
    currentPosition += local_multiplier;
  }
  if(n >= 7) {
    digitalWrite(pin1, HIGH);
    if(!safeDelay(delaytime)) return stepsCompleted;
    stepsCompleted++;
    currentPosition += local_multiplier;
  }
  return stepsCompleted;
}

int DS8PaiReverse(){
  int stepsCompleted = 0;
  int local_multiplier = (currentDirection == FORWARD) ? -1 : 1;
  
  digitalWrite(pin1, HIGH); // step1: (1,0,0,0)
  if(!safeDelay(delaytime)) return stepsCompleted;
  stepsCompleted++;
  currentPosition += local_multiplier;
  
  digitalWrite(pin4, HIGH); // step2: (1,0,0,1)
  if(!safeDelay(delaytime)) return stepsCompleted;
  stepsCompleted++;
  currentPosition += local_multiplier;
  
  digitalWrite(pin1, LOW);  // step3: (0,0,0,1)
  if(!safeDelay(delaytime)) return stepsCompleted;
  stepsCompleted++;
  currentPosition += local_multiplier;
  
  digitalWrite(pin3, HIGH); // step4: (0,0,1,1)
  if(!safeDelay(delaytime)) return stepsCompleted;
  stepsCompleted++;
  currentPosition += local_multiplier;
  
  digitalWrite(pin4, LOW);  // step5: (0,0,1,0)
  if(!safeDelay(delaytime)) return stepsCompleted;
  stepsCompleted++;
  currentPosition += local_multiplier;
  
  digitalWrite(pin2, HIGH); // step6: (0,1,1,0)
  if(!safeDelay(delaytime)) return stepsCompleted;
  stepsCompleted++;
  currentPosition += local_multiplier;
  
  digitalWrite(pin3, LOW);  // step7: (0,1,0,0)
  if(!safeDelay(delaytime)) return stepsCompleted;
  stepsCompleted++;
  currentPosition += local_multiplier;
  
  digitalWrite(pin1, HIGH); // step8: (1,1,0,0)
  if(!safeDelay(delaytime)) return stepsCompleted;
  stepsCompleted++;
  currentPosition += local_multiplier;
  
  digitalWrite(pin2, LOW);
  safeDelay(delaytime);
  
  return stepsCompleted;
}

int partialDS8PaiReverse(int n) {
  int stepsCompleted = 0;
  int local_multiplier = (currentDirection == FORWARD) ? -1 : 1;
  if(n >= 1) {
    digitalWrite(pin4, HIGH);
    if(!safeDelay(delaytime)) return stepsCompleted;
    stepsCompleted++;
    currentPosition += local_multiplier;
  }
  if(n >= 2) {
    digitalWrite(pin1, LOW);
    if(!safeDelay(delaytime)) return stepsCompleted;
    stepsCompleted++;
    currentPosition += local_multiplier;
  }
  if(n >= 3) {
    digitalWrite(pin3, HIGH);
    if(!safeDelay(delaytime)) return stepsCompleted;
    stepsCompleted++;
    currentPosition += local_multiplier;
  }
  if(n >= 4) {
    digitalWrite(pin4, LOW);
    if(!safeDelay(delaytime)) return stepsCompleted;
    stepsCompleted++;
    currentPosition += local_multiplier;
  }
  if(n >= 5) {
    digitalWrite(pin2, HIGH);
    if(!safeDelay(delaytime)) return stepsCompleted;
    stepsCompleted++;
    currentPosition += local_multiplier;
  }
  if(n >= 6) {
    digitalWrite(pin3, LOW);
    if(!safeDelay(delaytime)) return stepsCompleted;
    stepsCompleted++;
    currentPosition += local_multiplier;
  }
  if(n >= 7) {
    digitalWrite(pin1, HIGH);
    if(!safeDelay(delaytime)) return stepsCompleted;
    stepsCompleted++;
    currentPosition += local_multiplier;
  }
  return stepsCompleted;
}

// --------------------- Motor control ---------------------//

void rotateStepsForward(int steps, int mode) {
  int executedSteps = 0; 
  motorState = RUNNING;
  haltRequested = false; 
  
  if (mode == MODE_S4) {
    int cycles = steps / 4;     
    int remainder = steps % 4;   
    for (int i = 0; i < cycles; i++) {
      if (haltRequested) { 
        break;
      }
      int ret = S4Pai();  
      executedSteps += ret;
      if(ret < 4){ 
        motorState = IDLE;
        return;
      }
    }
    if (haltRequested) {
      motorState = IDLE;
      return;
    }
    if (remainder > 0) {
      int ret = partialS4Pai(remainder);
      executedSteps += ret;
    }
  }
  else if (mode == MODE_DS8) {
    int cycles = steps / 8;     
    int remainder = steps % 8;
    for (int i = 0; i < cycles; i++) {
      if (haltRequested) { 
        break;
      }
      int ret = DS8Pai();
      executedSteps += ret;
      if(ret < 8){
        motorState = IDLE;
        return;
      }
    }
    if (haltRequested) {
      motorState = IDLE;
      return;
    }
    if (remainder > 0) {
      int ret = partialDS8Pai(remainder);
      executedSteps += ret;
    }
  }

  resetPins();
  
  haltRequested = false;
  motorState = IDLE;
  savePositionToEEPROM();  // Save position to EEPROM
}


void rotateStepsReverse(int steps, int mode) {
  int executedSteps = 0;
  motorState = RUNNING;
  haltRequested = false;
  
  if (mode == MODE_S4) {
    int cycles = steps / 4;
    int remainder = steps % 4;
    for (int i = 0; i < cycles; i++) {
      if (haltRequested) { 
        break;
      }
      int ret = S4PaiReverse();
      executedSteps += ret;
      if(ret < 4){
        motorState = IDLE;
        return;
      }
    }
    if (haltRequested) {
      motorState = IDLE;
      return;
    }
    if (remainder > 0) {
      int ret = partialS4PaiReverse(remainder);
      executedSteps += ret;
    }
  }
  else if (mode == MODE_DS8) {
    int cycles = steps / 8;
    int remainder = steps % 8;
    for (int i = 0; i < cycles; i++) {
      if (haltRequested) { 
        break;
      }
      int ret = DS8PaiReverse();
      executedSteps += ret;
      if(ret < 8){
        motorState = IDLE;
        return;
      }
    }
    if (haltRequested) {
      motorState = IDLE;
      return;
    }
    if (remainder > 0) {
      int ret = partialDS8PaiReverse(remainder);
      executedSteps += ret;
    }
  }

  resetPins();
  
  haltRequested = false;
  motorState = IDLE;
  savePositionToEEPROM();  // Save position to EEPROM
}
