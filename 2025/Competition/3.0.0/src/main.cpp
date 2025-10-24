#include "vex.h"

// ---- START VEXCODE CONFIGURED DEVICES ----
// Robot Configuration:
// [Name]               [Type]        [Port(s)]
// Controller1          controller
// ArmMotor             motor         3
// ArmPot               pot           A
// ServoOC              servo         B
// ServoPush            servo         C
// LeftMotor            motor         1
// RightMotor           motor         2
// ---- END VEXCODE CONFIGURED DEVICES ----

using namespace vex;

// ================== VARIABLES ==================
int driveDeadband = 15;

int magOpen = 100;
int magClosed = 0;

int panelForward = 80;
int panelBack = 0;

double armLow = 3000;  // lowest 
double armMid = 2000;  // middle 
double armHigh = 700;  // top 
double armSpeed = 60;
double armTolerance = 50;

int armState = 0;  // 0=low, 1=mid, 2=high
bool armBusy = false;
double armTarget = 3000; // start position

bool magToggle = false;
bool panelToggle = false;

int armControlThread() {
  while (true) {
    double current = ArmPot.angle(degrees);
    double error = armTarget - current;

    if (fabs(error) > armTolerance) {
      armBusy = true;
      if (error > 0)
        ArmMotor.spin(forward, armSpeed, pct);
      else
        ArmMotor.spin(reverse, armSpeed, pct);
    } else {
      ArmMotor.stop(hold);
      armBusy = false;
    }

    wait(20, msec);
  }
  return 0;
}

void toggleMagAction() {
  magToggle = !magToggle;
  ServoOC.setPosition((magToggle) ? magOpen : magClosed, degrees);
}

void togglePanelAction() {
  panelToggle = !panelToggle;
  ServoPush.setPosition((panelToggle) ? panelForward : panelBack, degrees);
}

void cycleArmPosition() {
  if (armBusy) return;
  armState = (armState + 1) % 3;
  armTarget = (armState == 0) ? armLow :
              (armState == 1) ? armMid :
                                armHigh;
}

struct Action {
  bool lastState;
  void (*onPress)();
  const controller::button *button;

  void update() {
    bool now = button->pressing();
    if (now && !lastState && onPress)
      onPress();
    lastState = now;
  }
};

int main() {
  vexcodeInit();

  thread armThread(armControlThread);

  Action actions[4];
  actions[0] = {false, toggleMagAction, &Controller1.ButtonY};
  actions[1] = {false, togglePanelAction, &Controller1.ButtonA};
  actions[2] = {false, nullptr, nullptr};
  actions[3] = {false, cycleArmPosition, &Controller1.ButtonB};

  while (true) {
    int leftSpeed = Controller1.Axis3.position();
    int rightSpeed = Controller1.Axis2.position();

    if (abs(leftSpeed) < driveDeadband) leftSpeed = 0;
    if (abs(rightSpeed) < driveDeadband) rightSpeed = 0;

    LeftMotor.spin(forward, leftSpeed, pct);
    RightMotor.spin(forward, rightSpeed, pct);

    for (int i = 0; i < 4; ++i)
      actions[i].update();
  }
}
