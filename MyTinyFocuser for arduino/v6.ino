#include <Arduino.h>
#include "MotorControl.h"

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
      }
      else {
        Serial.print("recieve: ");
        Serial.println(input);
        Serial.println("wrong order, please use mode,position");
      }
    }
  }
}
