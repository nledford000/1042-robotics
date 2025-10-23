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

double armLow = 3000; // lowest 
double armMid = 2000; // middle 
double armHigh = 700; // top 
double armSpeed = 60;
double armTolerance = 50;

int armState = 0; // 0=low, 1=mid, 2=high
bool armBusy = false;
double armTarget = 0;


int moveArmTo(double target) { // NTS: If it's acting schizo, add tolerance
  if (armBusy) return 0;
  armBusy = true;

  double current = ArmPot.angle(degrees);
  while (fabs(current - target) > armTolerance) {
    current = ArmPot.angle(degrees);

    if (current < target)
      ArmMotor.spin(forward, armSpeed, pct);
    else
      ArmMotor.spin(reverse, armSpeed, pct);

    wait(20, msec);
  }

  ArmMotor.stop(hold);
  armBusy = false;
  return 0;
}

int moveArmThread() {
  moveArmTo(armTarget);
  return 0;
}

void cycleArmPosition() {
  if (armBusy) return;
  armState = (armState + 1) % 3;

  armTarget = (armState == 0) ? armLow :
              (armState == 1) ? armMid :
                                armHigh;

  // Launch thread
  thread t(moveArmThread);
}

bool magToggle = false;
bool panelToggle = false;

void toggleMagAction() {
  magToggle = !magToggle;
  ServoOC.setPosition((magToggle) ? magOpen : magClosed, degrees);
}

void togglePanelAction() {
  panelToggle = !panelToggle;
  ServoPush.setPosition((panelToggle) ? panelForward : panelBack, degrees);
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

  Action actions[4];
  actions[0] = {false, toggleMagAction, &Controller1.ButtonY};
  actions[1] = {false, togglePanelAction, &Controller1.ButtonA};
//  actions[2] = {false, nullptr, &Controller1.ButtonX};
  actions[3] = {false, cycleArmPosition, &Controller1.ButtonB};

  while (true) {
    int leftSpeed = Controller1.Axis3.position();
    int rightSpeed = Controller1.Axis2.position();

    LeftMotor.spin(forward, (abs(leftSpeed) >= driveDeadband) ? leftSpeed : 0, pct);
    RightMotor.spin(forward, (abs(rightSpeed) >= driveDeadband) ? rightSpeed : 0, pct);

    for (int i = 0; i < 4; ++i)
      actions[i].update();
  }
}
