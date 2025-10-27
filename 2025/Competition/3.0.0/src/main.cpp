#include "vex.h"

using namespace vex;

const int driveDeadband = 15;
const double armLow = 223; 
const double armMid = 60;
const double armHigh = 1;
const double armUpSpeed = 100;
const double armDownSpeed = 60;
const double armTolerance = 5;
const int magOpen = 100;
const int magClosed = 0;
const double neuralPos1 = 0;//0.67, 67, 151, 250
const double neuralPos2 = 67;
const double neuralPos3 = 151;
const double neuralPos4 = 250;
const double neuralUpSpeed = 80;
const double neuralDownSpeed = 60;
const double neuralTolerance = 5;

int armState = 1;
double armTarget = armLow;
bool armAutoMove = false;
bool magToggle = false;
int neuralState = 1;
double neuralTarget = neuralPos1;
bool neuralAutoMove = false;

void toggleMagAction() {
  magToggle = !magToggle;
  ServoOC.setPosition(magToggle ? magOpen : magClosed, degrees);
}

void cycleArmPosition() {
  armState++;
  if (armState > 3) armState = 1;
  if (armState == 1) armTarget = armLow;
  else if (armState == 2) armTarget = armMid;
  else if (armState == 3) armTarget = armHigh;
  armAutoMove = true;
}

void cycleNeuralPosition() {
  neuralState++;
  if (neuralState > 4) neuralState = 1;
  if (neuralState == 1) neuralTarget = neuralPos1;
  else if (neuralState == 2) neuralTarget = neuralPos2;
  else if (neuralState == 3) neuralTarget = neuralPos3;
  else if (neuralState == 4) neuralTarget = neuralPos4;
  neuralAutoMove = true;
}

void armUpManual() {
  ArmMotor.spin(forward, armUpSpeed, pct);
  armAutoMove = false;
}

void armDownManual() {
  ArmMotor.spin(reverse, armDownSpeed, pct);
  armAutoMove = false;
}

bool buttonYPressed() { return Controller1.ButtonY.pressing(); }
bool buttonXPressed() { return Controller1.ButtonX.pressing(); }
bool buttonBPressed() { return Controller1.ButtonB.pressing(); }
bool buttonR1Pressed() { return Controller1.ButtonR1.pressing(); }
bool buttonR2Pressed() { return Controller1.ButtonR2.pressing(); }

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
        ArmMotor.spin(error > 0 ? reverse : forward,
                      (error > 0 ? armDownSpeed : armUpSpeed), pct);
      } else {
        ArmMotor.stop(hold);
        armAutoMove = false;
      }
    } else if (!Controller1.ButtonR1.pressing() && !Controller1.ButtonR2.pressing()) {
      ArmMotor.stop(hold);
    }
    this_thread::sleep_for(20);
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
    this_thread::sleep_for(20);
  }
  return 0;
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
    {false, armUpManual, buttonR1Pressed}
  };
  while (true) {
    int leftSpeed = Controller1.Axis3.position();
    int rightSpeed = Controller1.Axis2.position();
    if (abs(leftSpeed) < driveDeadband) leftSpeed = 0;
    if (abs(rightSpeed) < driveDeadband) rightSpeed = 0;
    LeftMotor.spin(forward, leftSpeed, pct);
    RightMotor.spin(forward, rightSpeed, pct);
    for (int i = 0; i < sizeof(actions)/sizeof(actions[0]); i++)
      actions[i].update();
    this_thread::sleep_for(20);
  }
}
