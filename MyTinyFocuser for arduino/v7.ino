#include <Arduino.h>
#include "MotorControl.h"
#include <EEPROM.h>

// EEPROM address allocation (Arduino usually has 1KB EEPROM)
#define EEPROM_POSITION_ADDR 0      // 4 bytes: currentPosition (long)
#define EEPROM_DIRECTION_ADDR 4     // 1 byte: currentDirection (0=FORWARD, 1=BACKWARD)
#define EEPROM_MOVINGTYPE_ADDR 5    // 1 byte: movingType (0=ABS, 1=REL)
#define EEPROM_MODE_ADDR 6          // 1 byte: currentMode (0=MODE_DS8, 1=MODE_S4)
#define EEPROM_VALID_ADDR 7         // 1 byte: validity flag (0xAB = valid data)

#define EEPROM_VALID_FLAG 0xAB

// Save all settings to EEPROM
void saveAllSettingsToEEPROM() {
  // Save position (4 bytes)
  for (int i = 0; i < 4; i++) {
    EEPROM.write(EEPROM_POSITION_ADDR + i, (byte)((currentPosition >> (8 * i)) & 0xFF));
  }
  
  // Save direction (1 byte)
  EEPROM.write(EEPROM_DIRECTION_ADDR, (byte)currentDirection);
  
  // Save moving type (1 byte)
  EEPROM.write(EEPROM_MOVINGTYPE_ADDR, (byte)movingType);
  
  // Save motor mode (1 byte)
  EEPROM.write(EEPROM_MODE_ADDR, (byte)currentMode);
  
  // Set validity flag
  EEPROM.write(EEPROM_VALID_ADDR, EEPROM_VALID_FLAG);
  
  Serial.print("Settings saved - Position:");
  Serial.print(currentPosition);
  Serial.print(" Direction:");
  Serial.print(currentDirection);
  Serial.print(" Moving type:");
  Serial.print(movingType);
  Serial.print(" Mode:");
  Serial.println(currentMode);
}

// Load all settings from EEPROM
void loadAllSettingsFromEEPROM() {
  // Check validity flag
  if (EEPROM.read(EEPROM_VALID_ADDR) != EEPROM_VALID_FLAG) {
    Serial.println("EEPROM data invalid, using default configuration");
    currentPosition = 5000;
    currentDirection = FORWARD;
    movingType = ABS;
    currentMode = MODE_DS8;
    saveAllSettingsToEEPROM();
    return;
  }
  
  // Read position (4 bytes)
  long savedPosition = 0;
  for (int i = 0; i < 4; i++) {
    savedPosition |= ((long)EEPROM.read(EEPROM_POSITION_ADDR + i)) << (8 * i);
  }
  
  // Validate position range
  if (savedPosition >= 0 && savedPosition <= maxstep) {
    currentPosition = savedPosition;
  } else {
    currentPosition = 5000;
  }
  
  // Read direction
  byte savedDirection = EEPROM.read(EEPROM_DIRECTION_ADDR);
  if (savedDirection == FORWARD || savedDirection == BACKWARD) {
    currentDirection = (MotorDirection)savedDirection;
  } else {
    currentDirection = FORWARD;
  }
  
  // Read moving type
  byte savedMovingType = EEPROM.read(EEPROM_MOVINGTYPE_ADDR);
  if (savedMovingType == ABS || savedMovingType == REL) {
    movingType = (MovingType)savedMovingType;
  } else {
    movingType = ABS;
  }
  
  // Read motor mode
  byte savedMode = EEPROM.read(EEPROM_MODE_ADDR);
  if (savedMode == MODE_DS8 || savedMode == MODE_S4) {
    currentMode = (MotorMode)savedMode;
  } else {
    currentMode = MODE_DS8;
  }
  
  Serial.print("Settings restored from EEPROM - Position:");
  Serial.print(currentPosition);
  Serial.print(" Direction:");
  Serial.print(currentDirection);
  Serial.print(" Moving type:");
  Serial.print(movingType);
  Serial.print(" Mode:");
  Serial.println(currentMode);
}

// Backward compatible functions
void loadPositionFromEEPROM() {
  loadAllSettingsFromEEPROM();
}

void savePositionToEEPROM() {
  saveAllSettingsToEEPROM();
}

void setup(){
  pinMode(pin1, OUTPUT);
  pinMode(pin2, OUTPUT);
  pinMode(pin3, OUTPUT);
  pinMode(pin4, OUTPUT);
  digitalWrite(pin1, LOW);
  digitalWrite(pin2, LOW);
  digitalWrite(pin3, LOW);
  digitalWrite(pin4, LOW);
  
  Serial.begin(9600);
  
  // Load all saved settings from EEPROM during initialization
  loadAllSettingsFromEEPROM();
}

void loop(){

  if (Serial.available() > 0) {
    String input = Serial.readStringUntil('\n');
    input.trim();
    if(input.length() > 0) {
      
      if (input.equalsIgnoreCase("connect")) {
        Serial.println("ThisAMyTinyFocuserBylewcherwyn");
      }
      else if(input.equalsIgnoreCase("isMoving")){
        Serial.println(motorState);
      }
      else if(input.equalsIgnoreCase("TempComp")){
        Serial.println(tempComp);
      }
      else if(input.equalsIgnoreCase("Temperature")){
        Serial.println(20);
      }
      else if(input.equalsIgnoreCase("maxstep")) {
        Serial.println(maxstep);
      } 
      else if(input.equalsIgnoreCase("current:position")){
        Serial.println(currentPosition);
      }
      else if(input.equalsIgnoreCase("currentState")){
        Serial.println(String(currentPosition) + ";" + String(motorState) + ";" + String(currentMode));
      }
      else if(input.equalsIgnoreCase("deviceState")){
        String output = String(maxstep) + ";" + String(movingType) + ";" + String(currentMode) + ";" + String(currentDirection);
        Serial.println(output);
      }
      else if(input.equalsIgnoreCase("set2middle")) {
          Serial.print("current position:");
          Serial.println(currentPosition);
          currentPosition = 5000;
          savePositionToEEPROM();  // Save new position
          Serial.print("update to 5000:");
          Serial.println(currentPosition);
      }

      else if(input.startsWith("mode:")) {
        int colonIndex = input.indexOf(':');
        String modeStr = input.substring(colonIndex + 1);
        modeStr.trim();
        int newMode = modeStr.toInt();
        if(newMode == MODE_S4 || newMode == MODE_DS8) {
          currentMode = (MotorMode)newMode;
          Serial.print("mode: ");
          Serial.println(currentMode);
          saveAllSettingsToEEPROM();  // Save settings
        } else {
          Serial.println("unsupported mode");
        }
      }
      else if(input.startsWith("direction:")) {
        int colonIndex = input.indexOf(':');
        String directionStr = input.substring(colonIndex + 1);
        directionStr.trim();
        int newDirection = directionStr.toInt();
        if(newDirection == FORWARD || newDirection == BACKWARD) {
          currentDirection = (MotorDirection)newDirection;
          Serial.print("direction: ");
          Serial.println(currentDirection);
          saveAllSettingsToEEPROM();  // Save settings
        } else {
          Serial.println("unsupported direction");
        }
      }
      else if(input.startsWith("maxstep:")) {
        int colonIndex = input.indexOf(':');
        String maxstepStr = input.substring(colonIndex + 1);
        maxstepStr.trim();
        maxstep = maxstepStr.toInt();
      }
      else if(input.startsWith("movingtype:")) {
        int colonIndex = input.indexOf(':');
        String movingTypeStr = input.substring(colonIndex + 1);
        movingTypeStr.trim();
        int newMovingType = movingTypeStr.toInt();
        if(newMovingType == ABS || newMovingType == REL) {
          movingType = (MovingType)newMovingType;
          Serial.print("moving type: ");
          Serial.println(movingType);
          saveAllSettingsToEEPROM();  // Save settings
        } else {
          Serial.println("unsupported moving type");
        }
      }

      else if(input.startsWith("position:")) {
        int colonIndex = input.indexOf(':');
        String targetStr = input.substring(colonIndex + 1);
        targetStr.trim();
        Serial.print("target: ");
        Serial.println(targetStr);
        Serial.print("current: ");
        Serial.println(currentPosition);
        long delta = 0;
        if (movingType == ABS) {
          long targetPos = targetStr.toInt();
          delta = targetPos - currentPosition;
        }
        else if (movingType == REL) {
          delta = targetStr.toInt();
        }
        
        if (currentDirection == FORWARD) {
          if(delta >= 0) {
            rotateStepsForward(delta, currentMode);
          } else  {
            rotateStepsReverse(-delta, currentMode);
          } 
        } else if (currentDirection == BACKWARD) { 
          if(delta >= 0) {
            rotateStepsReverse(delta, currentMode);
          } else {
            rotateStepsForward(-delta, currentMode);
          } 
        } else{
          Serial.println("wrong direction");
        }
        Serial.println(currentPosition);
      }

      else if(input.equalsIgnoreCase("halt")) {
          Serial.println("halt received");
          motorState = IDLE;
          savePositionToEEPROM();  // Immediately save current position
      }
      else {
        Serial.print("recieve: ");
        Serial.println(input);
        Serial.println("wrong order, please use mode,position");
      }
    }
  }
}
