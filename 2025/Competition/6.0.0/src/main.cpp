#include "vex.h"
#include <cmath>
using namespace vex;

const int driveDeadband = 15;
const double armLow = 186;
const double armHigh = 230;
const double armUpSpeed = 90;
const double armDownSpeed = 90;
const double armTolerance = 1;

const int magOpen = 0;
const int magClosed = 28;

const double neuralPos3 = 194;
const double neuralPos2 = 221;
const double neuralPos1 = 250;

const double neuralUpSpeed = 60;
const double neuralDownSpeed = 60;
const double neuralTolerance = 2;

const double gravK = 15.0;
const double ANGLE_OFFSET = 190.0;

const double TILE_SERVO_INCREMENT = 13;
double tileServoPosition = -25;
const double TILE_SERVO_INIT = -25;
const double TILE_SERVO_MAX = 34;
const double STICK_SERVO_INIT = 0;
double servoStickMax = 200;

int armState = 1;
double armTarget = armLow;
bool armAutoMove = false;
bool magToggle = false;
int neuralState = 1;
double neuralTarget = neuralPos1;
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
  return gravK * fabs(cos(angleRad));
}

void toggleMagAction() {
  magToggle = !magToggle;
  ServoOC.setPosition(magToggle ? magOpen : magClosed, degrees);
}

void cycleArmPosition() {
  armState++;
  if (armState > 2) armState = 1;
  if (armState == 1) armTarget = armLow;
  else if (armState == 2) armTarget = armHigh;
  armAutoMove = true;
}

void cycleNeuralPosition() {
  neuralState++;
  if (neuralState > 3)
    neuralState = 1;

  if (neuralState == 1)
    neuralTarget = neuralPos1;
  else if (neuralState == 2)
    neuralTarget = neuralPos2;
  else if (neuralState == 3)
    neuralTarget = neuralPos3;

  neuralAutoMove = true;
}

void armUpManual() {
  double grav = getGravityAssistPct();
  double out = clampPct(armUpSpeed + grav);  
  if (out < 10) out = 10;
  ArmMotor.spin(forward, out, pct);
  armAutoMove = false;
  armInPosition = false;
}

void armDownManual() {
  double grav = getGravityAssistPct();
  double out = clampPct(armDownSpeed - grav);
  if (out < 10) out = 10;// xyz...
  ArmMotor.spin(reverse, out, pct);
  armAutoMove = false;
  armInPosition = false;
}

void incrementTileServoArm() {
  tileServoPosition += TILE_SERVO_INCREMENT;
    if (tileServoPosition > TILE_SERVO_MAX) {
      tileServoPosition = TILE_SERVO_INIT;
      Controller1.Screen.clearScreen();
      Controller1.Screen.print("LAST");
    } else if (tileServoPosition + TILE_SERVO_INCREMENT > TILE_SERVO_MAX){
      tileServoPosition +=  8;
      Controller1.Screen.clearScreen();
      Controller1.Screen.print("ALMOST LAST");
    }

  ServoInc.setPosition(tileServoPosition, degrees);
  // Controller1.Screen.clearScreen();
  // Controller1.Screen.print("%d", tileServoPosition);
}

void toggleServoStickAction1() {
    servoStick.setPosition(STICK_SERVO_INIT, rotationUnits::deg);
}
void toggleServoStickAction2() {
    servoStick.setPosition(servoStickMax, rotationUnits::deg);
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
      double current = ArmPot.value(deg);
      double error = armTarget - current;
      if (fabs(error) > armTolerance) {
        armInPosition = false;
        bool needUp = (error > 0);
        double grav = getGravityAssistPct();
        double base = needUp ? armUpSpeed : armDownSpeed;
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
      double error = neuralTarget - current;
      if (fabs(error) > neuralTolerance) {
        NeuralMotor.spin(error > 0 ? reverse : forward,
                         (error > 0 ? neuralDownSpeed : neuralUpSpeed), pct);
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
    ServoOC.setPosition(magOpen, degrees);// here
    armTarget = armHigh;
    armAutoMove = true;
    macroStep = 2;
  }
  else if (macroStep == 2) {
    if (armInPosition) {
      magToggle = false;
      ServoOC.setPosition(magClosed, degrees);// here
      wait(400, msec);
      armTarget = armLow;
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
  //Controller1.Screen.setCursor(1, 1);
  //Controller1.Screen.print("%d", magToggle);
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
  // Controller1.Screen.clearScreen();
  // Controller1.Screen.print("%d", tileServoPosition);

    int leftSpeed = Controller1.Axis3.position();
    int rightSpeed = Controller1.Axis2.position();
    if (abs(leftSpeed) < driveDeadband) leftSpeed = 0;
    if (abs(rightSpeed) < driveDeadband) rightSpeed = 0;
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
