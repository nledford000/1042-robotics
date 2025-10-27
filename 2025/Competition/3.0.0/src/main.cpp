#include "vex.h"

using namespace vex;

//CONSTANTS
const int driveDeadband = 15;
const int magOpen = 100;
const int magClosed = 0;
const int panelForward = 80;
const int panelBack = 0;

const double armLow = 225;
const double armMid = 125;
const double armHigh = 75;
const double armSpeed = 60;
const double armTolerance = 5;

//STATE VARIABLES
int armState = 0;
double armTarget = armLow;
bool armAutoMove = false;
bool magToggle = false;
bool panelToggle = false;
bool neuralToggle = false;

double clamp(double value, double minVal, double maxVal) {
  return value < minVal ? minVal : (value > maxVal ? maxVal : value);
}

void toggleMagAction() {
  magToggle = !magToggle;
  ServoOC.setPosition(magToggle ? magOpen : magClosed, degrees);
}

void togglePanelAction() {
  panelToggle = !panelToggle;
  ServoPush.setPosition(panelToggle ? panelForward : panelBack, degrees);
}

void toggleNeuralMotor() {
  neuralToggle = !neuralToggle;
  if (neuralToggle)
    NeuralMotor.spin(forward, 100, pct);
  else
    NeuralMotor.stop(coast);
}

void cycleArmPosition() {
  armState++;
  if (armState > 3) armState = 1;

  if (armState == 1) armTarget = armLow;
  else if (armState == 2) armTarget = armMid;
  else if (armState == 3) armTarget = armHigh;

  armAutoMove = true;
}

bool buttonYPressed() { return Controller1.ButtonY.pressing(); }
bool buttonAPressed() { return Controller1.ButtonA.pressing(); }
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
    if (now && !lastState && onPress) onPress();
    lastState = now;
  }
};

int armAutoThread() {
  while (true) {
    if (armAutoMove) {
      double current = ArmPot.value(deg);
      double error = armTarget - current;

      if (fabs(error) > armTolerance) {
        ArmMotor.spin(error > 0 ? reverse : forward, armSpeed, pct);
      } else {
        ArmMotor.stop(hold);
        armAutoMove = false;
      }
    } else {
      ArmMotor.stop(hold);
    }
  }
  return 0;
}

void armUpManual() {
  ArmMotor.spin(reverse, armSpeed, pct);
  armAutoMove = false;
}

void armDownManual() {
  ArmMotor.spin(forward, armSpeed, pct);
  armAutoMove = false; 
}

int main() {
  vexcodeInit();

  thread armThread(armAutoThread);

  Action actions[] = {
    {false, toggleMagAction, buttonYPressed},
    {false, togglePanelAction, buttonAPressed},
    {false, toggleNeuralMotor, buttonXPressed},
    {false, cycleArmPosition, buttonBPressed},
    {false, armUpManual, buttonR1Pressed},
    {false, armDownManual, buttonR2Pressed}
  };

  while (true) {
    int leftSpeed = Controller1.Axis3.position();
    int rightSpeed = Controller1.Axis2.position();

    if (abs(leftSpeed) < driveDeadband) leftSpeed = 0;
    if (abs(rightSpeed) < driveDeadband) rightSpeed = 0;

    LeftMotor.spin(forward, leftSpeed, pct);
    RightMotor.spin(forward, rightSpeed, pct);

    for (int i = 0; i < 6; i++) actions[i].update();
  }
}
