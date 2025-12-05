#include "vex.h"
#include <cmath>
using namespace vex;

const int DRIVE_DEADBAND = 15;
const int MAG_OPEN = 0;
const int MAG_CLOSED = 28;
const double ARM_LOW = 195;//186>>195
const double ARM_HIGH = 249;//230>>249
const double ARM_UP_SPEED = 90;
const double ARM_DOWN_SPEED = 90;
const double ARM_TOLERANCE = 1;
const double NEURAL_POS_3 = 50;// 194>>50
const double NEURAL_POS_2 = 115;// 221>>115
const double NEURAL_POS_1 = 215;// 250>>215
const double NEURAL_UP_SPEED = 60;
const double NEURAL_DOWN_SPEED = 60;
const double NEURAL_TOLERANCE = 2;
const double GRAV_K = 15.0;
const double ANGLE_OFFSET = 190.0;
const double TILE_SERVO_INCREMENT = 13;
const double TILE_SERVO_INIT = -25;
const double TILE_SERVO_MAX = 34;
const double STICK_SERVO_INIT = -15;
const double STICK_SERVO_MAX = 150;

double tileServoPosition = -25;
int armState = 1;
double armTarget = ARM_LOW;
bool armAutoMove = false;
bool magToggle = false;
int neuralState = 1;
double neuralTarget = NEURAL_POS_1;
bool neuralAutoMove = false;
bool autonomousMode = false;
bool armInPosition = false;
int macroStep = 0;
bool armInLow;

double clampPct(double v) {
  if (v > 100.0) return 100.0;
  if (v < -100.0) return -100.0;
  return v;
}

double getGravityAssistPct() {
  const double PI = 3.1415926535896;
  double angleDeg = ArmPot.value(deg);
  double angleRad = (angleDeg - ANGLE_OFFSET) * PI / 180.0;
  return GRAV_K * fabs(cos(angleRad));
}

void toggleMagAction() {
  magToggle = !magToggle;
  ServoOC.setPosition(magToggle ? MAG_OPEN : MAG_CLOSED, degrees);
}

void cycleArmPosition() {
  armState++;
  if (armState > 2) armState = 1;
  if (armState == 1) armTarget = ARM_LOW;
  else if (armState == 2) armTarget = ARM_HIGH;
  armAutoMove = true;
}

void cycleNeuralPosition() {
  neuralState++;
  if (neuralState > 3)
    neuralState = 1;

  if (neuralState == 1)
    neuralTarget = NEURAL_POS_1;
  else if (neuralState == 2)
    neuralTarget = NEURAL_POS_2;
  else if (neuralState == 3)
    neuralTarget = NEURAL_POS_3;

  neuralAutoMove = true;
}

void armUpManual() {
  double grav = getGravityAssistPct();
  double out = clampPct(ARM_UP_SPEED + grav);  
  if (out < 10) out = 10;
  ArmMotor.spin(forward, out, pct);
  armAutoMove = false;
  armInPosition = false;
}

void armDownManual() {
  double grav = getGravityAssistPct();
  double out = clampPct(ARM_DOWN_SPEED - grav);
  if (out < 10) out = 10;
  ArmMotor.spin(reverse, out, pct);
  armAutoMove = false;
  armInPosition = false;
}

void incrementTileServoArm() {
  tileServoPosition += TILE_SERVO_INCREMENT;
    if (tileServoPosition > TILE_SERVO_MAX) {
      tileServoPosition = TILE_SERVO_INIT;
      Controller1.Screen.clearScreen();
    } else if (tileServoPosition + TILE_SERVO_INCREMENT > TILE_SERVO_MAX){
      tileServoPosition +=  8;
      Controller1.Screen.clearScreen();
    }

  ServoInc.setPosition(tileServoPosition, degrees);
}

void toggleServoStickAction1() {
    servoStick.setPosition(STICK_SERVO_INIT, rotationUnits::deg);
}

void toggleServoStickAction2() {
    servoStick.setPosition(STICK_SERVO_MAX, rotationUnits::deg);
}

bool buttonYPressed()  { return Controller1.ButtonY.pressing(); }
bool buttonXPressed()  { return Controller1.ButtonX.pressing(); }
bool buttonBPressed()  { return Controller1.ButtonB.pressing(); }
bool buttonR1Pressed() { return Controller1.ButtonR1.pressing(); }
bool buttonR2Pressed() { return Controller1.ButtonR2.pressing(); }
bool buttonAPressed()  { return Controller1.ButtonA.pressing(); }
bool buttonUpPressed() { return Controller1.ButtonUp.pressing(); }
bool buttonL1Pressed() { return Controller1.ButtonL1.pressing(); }
bool buttonL2Pressed() { return Controller1.ButtonL2.pressing(); }

struct Action {
  bool lastState;
  void (*onPress)();
  bool (*isPressed)();
  void update() {
    bool now = isPressed();
    if (now && !lastState && onPress)
      onPress();
    lastState = now;
  }
};

int armAutoThread() {
  while (true) {
    if (armAutoMove) {
      toggleServoStickAction1();
      double current = ArmPot.value(deg);
      double armPosition = armTarget - current;
      if (fabs(armPosition) > ARM_TOLERANCE) {
        armInPosition = false;
        bool needUp = (armPosition > 0);
        double grav = getGravityAssistPct();
        double base = needUp ? ARM_UP_SPEED : ARM_DOWN_SPEED;
        double out = needUp ? clampPct(base + grav) : clampPct(base - grav);
        if (!needUp && out < 10) out = 10;
        if (needUp) ArmMotor.spin(forward, out, pct);
        else ArmMotor.spin(reverse, out, pct);
      } else {
        ArmMotor.stop(hold);
        armAutoMove = false;
        armInPosition = true;
      }
    } else if (!Controller1.ButtonR1.pressing() && !Controller1.ButtonR2.pressing()) {
      ArmMotor.stop(hold);
    }
  }
  return 0;
}

int neuralAutoThread() {
  while (true) {
    if (neuralAutoMove) {
      double current = NeuralPot.value(deg);
      double neuralPosition = neuralTarget - current;
      if (fabs(neuralPosition) > NEURAL_TOLERANCE) {
        NeuralMotor.spin(neuralPosition > 0 ? reverse : forward,
                         (neuralPosition > 0 ? NEURAL_DOWN_SPEED : NEURAL_UP_SPEED), pct);
      } else {
        NeuralMotor.stop(hold);
        neuralAutoMove = false;
      }
    } else {
      NeuralMotor.stop(hold);
    }
  }
  return 0;
}

void macroSequenceStart() {
  autonomousMode = true;
  macroStep = 1;
  armInPosition = false;
}

void incrementTileServoArmAuto() {
  wait(2000, msec);
  incrementTileServoArm();
}

void macroSequenceUpdate() {
  if (macroStep == 1) {
    magToggle = true;
    ServoOC.setPosition(MAG_OPEN, degrees);
    armTarget = ARM_HIGH;
    armAutoMove = true;
    macroStep = 2;
  }
  else if (macroStep == 2) {
    if (armInPosition) {
      magToggle = false;
      ServoOC.setPosition(MAG_CLOSED, degrees);
      wait(400, msec);
      armTarget = ARM_LOW;
      armAutoMove = true;
      macroStep = 3;
    }
  }
  else if (macroStep == 3) {
    autonomousMode = false;
    macroStep = 0;
    thread incrementTileArm(incrementTileServoArmAuto);
  }
}

int main() {
  vexcodeInit();
  thread armThread(armAutoThread);
  thread neuralThread(neuralAutoThread);
  Action actions[] = {
    {false, toggleMagAction, buttonYPressed},
    {false, cycleNeuralPosition, buttonXPressed},
    {false, cycleArmPosition, buttonBPressed},
    {false, armDownManual, buttonR2Pressed},
    {false, armUpManual, buttonR1Pressed},
    {false, incrementTileServoArm, buttonAPressed},
    {false, macroSequenceStart, buttonUpPressed},
    {false, toggleServoStickAction1, buttonL1Pressed},
    {false, toggleServoStickAction2, buttonL2Pressed}
  };

  ServoInc.setPosition(TILE_SERVO_INIT, degrees);
  servoStick.setPosition(STICK_SERVO_INIT, degrees);

  while (true) {
    
    int leftSpeed = Controller1.Axis3.position();
    int rightSpeed = Controller1.Axis2.position();
    if (abs(leftSpeed) < DRIVE_DEADBAND) leftSpeed = 0;
    if (abs(rightSpeed) < DRIVE_DEADBAND) rightSpeed = 0;
    LeftMotor.spin(forward, leftSpeed, pct);
    RightMotor.spin(forward, rightSpeed, pct);
    for (int i = 0; i < sizeof(actions)/sizeof(actions[0]); i++) {
      actions[i].update();
    }
    if (autonomousMode) {
      macroSequenceUpdate();
    }
  }
}
