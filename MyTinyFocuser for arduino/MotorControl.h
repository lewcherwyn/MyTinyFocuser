#ifndef MOTORCONTROL_H
#define MOTORCONTROL_H

#include <Arduino.h>

// --------------------- 电机控制代码 ---------------------

// 定义引脚及延时
extern int pin1; // 蓝
extern int pin2; // 粉
extern int pin3; // 黄
extern int pin4; // 橙
extern int delaytime; // 延时 (ms)

// MovingType
enum MovingType {
  ABS,
  REL
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
  MODE_DS8,
  MODE_S4
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

// 当前步数位置，初始设置为5000
extern long currentPosition;
extern long maxstep; 
extern long maxIncrement; 
extern int multiplier;

// halt 指令标志，用于在电机运动过程中检测到串口输入 halt 指令
extern volatile bool haltRequested;

// 电机控制函数声明
void rotateStepsForward(int steps, int mode);
void rotateStepsReverse(int steps, int mode);

// -------------------【双4拍模式】-------------------//
// 正向全周期（4步）
int S4Pai();
// 正向部分步（1～3步）
int partialS4Pai(int n);
// 反向全周期（4步）
int S4PaiReverse();
// 反向部分步（1～3步）
int partialS4PaiReverse(int n);

// -------------------【单双8拍模式】-------------------//
// 正向全周期（8步）
int DS8Pai();
// 正向部分步（1～7步）
int partialDS8Pai(int n);
// 反向全周期（8步）
int DS8PaiReverse();
// 反向部分步（1～7步）
int partialDS8PaiReverse(int n);


// 安全延时函数：将 totalDelay 拆分为 1ms 的延时，并在期间轮询串口输入 halt 指令
bool safeDelay(int totalDelay);

#endif
