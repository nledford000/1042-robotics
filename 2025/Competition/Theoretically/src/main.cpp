#include "vex.h"

// ---- START VEXCODE CONFIGURED DEVICES ----
// Robot Configuration:
// [Name]               [Type]        [Port(s)]
// Controller1          controller                    
// LeftMotor            motor         1               
// RightMotor           motor         2               
// ArmMotor             motor         3               
// ServoOC              servo         A               
// ServoInc             servo         C               
// NeuralPot            pot           G               
// NeuralMotor          motor         4               
// ArmPot               pot           E               
// servoStick           servo         F               
// ---- END VEXCODE CONFIGURED DEVICES ----
#include <cmath>
using namespace vex;

// ---------------- Constants ----------------
const int driveDeadband = 15;

const double armLow = 180;
const double armHigh = 230;
const double armUpSpeed = 100;
const double armDownSpeed = 100;
const double armTolerance = 1;

const int magOpen = 0;
const int magClosed = 100;

const double neuralPos1 = 194;
const double neuralPos2 = 221;
const double neuralPos3 = 250;

const double neuralUpSpeed = 60;
const double neuralDownSpeed = 60;
const double neuralTolerance = 2;

const double gravK = 15.0;
const double angleOffset = 204.0;

double servoIncValue = 15;
double servoIncPos = -25; 
double servoInit = -25;
double servoMax = 70;
double servoStickInit = -80;
double servoStickMax = 180;
// ---------------- State Variables ----------------
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

// ---------------- Actions ----------------
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
  if (neuralState > 3) neuralState = 1;
  if (neuralState == 1) neuralTarget = neuralPos1;
  else if (neuralState == 2) neuralTarget = neuralPos2;
  else if (neuralState == 3) neuralTarget = neuralPos3;
  neuralAutoMove = true;
}

double clampPct(double v) {
  if (v > 100.0) return 100.0;
  if (v < -100.0) return -100.0;
  return v;
}

double getGravityAssistPct() {
  const double PI = 3.1415926535896;
  double angleDeg = ArmPot.value(deg);
  double angleRad = (angleDeg - angleOffset) * PI / 180.0;
  return gravK * fabs(cos(angleRad));
}

void armUpManual() {
  double grav = getGravityAssistPct();
  double out = clampPct(armUpSpeed + grav);
  ArmMotor.spin(forward, out, pct);
  armAutoMove = false;
}

void armDownManual() {
  double grav = getGravityAssistPct();
  double out = clampPct(armDownSpeed - grav);
  if (out < 10) out = 10;
  ArmMotor.spin(reverse, out, pct);
  armAutoMove = false;
}

void cycleServoInc() {
  servoIncPos += servoIncValue;
  if (servoIncPos > servoMax) {
    servoIncPos = servoInit;
  }
  ServoInc.setPosition(servoIncPos, degrees);
}

void toggleServoStickAction() {
  static bool stickToggle;
  stickToggle = !stickToggle;
  if (stickToggle) {
    servoStick.setPosition(servoStickMax, degrees);
  } else {
    servoStick.setPosition(servoStickInit, degrees);
  }
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

// ---------------- Action Struct ----------------
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

// ---------------- Threads ----------------
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

// ---------------- Macro Sequence ----------------
void macroSequenceStart() {
  autonomousMode = true;
  macroStep = 1;
}

void macroSequenceUpdate() {
  if (macroStep == 1) {
    magToggle = true;
    ServoOC.setPosition(magOpen, degrees);
    armTarget = armHigh;
    armAutoMove = true;
    macroStep = 2; 
    wait(300, msec);
  } 
  else if (macroStep == 2) {
    if (armInPosition) {
      magToggle = false;
      ServoOC.setPosition(magClosed, degrees);
      armTarget = armLow;
      armAutoMove = true;
      macroStep = 3;
      wait(300, msec);
    }
  } 
  else if (macroStep == 3) {
    if (armInPosition) {
      servoIncPos -= servoIncValue;
      if (servoIncPos < 0) servoIncPos = 180;
      ServoInc.setPosition(servoIncPos, degrees);

      autonomousMode = false;
      macroStep = 0;
    }
    wait(300, msec);
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
    {false, cycleServoInc, buttonAPressed},
    {false, macroSequenceStart, buttonUpPressed},
    {false, toggleServoStickAction, buttonL1Pressed}, // L1 toggles servoStick
    {false, toggleServoStickAction, buttonL2Pressed}  // L2 also toggles servoStick
  };

  ServoInc.setPosition(servoInit, degrees);
  servoStick.setPosition(servoStickInit, degrees);

  while (true) {
    int leftSpeed = Controller1.Axis3.position();
    int rightSpeed = Controller1.Axis2.position();
    if (abs(leftSpeed) < driveDeadband) leftSpeed = 0;
    if (abs(rightSpeed) < driveDeadband) rightSpeed = 0;
    LeftMotor.spin(forward, leftSpeed, pct);
    RightMotor.spin(forward, rightSpeed, pct);


    for (int i = 0; i < sizeof(actions)/sizeof(actions[0]); i++)
      actions[i].update();

    if (autonomousMode) {
      macroSequenceUpdate();
    }
  }
}
