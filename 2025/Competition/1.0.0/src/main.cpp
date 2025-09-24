#include "vex.h"

// ---- START VEXCODE CONFIGURED DEVICES ----
// Robot Configuration:
// [Name]               [Type]        [Port(s)]
// Controller1          controller                    
// LeftMotor            motor         1               
// RightMotor           motor         2               
// ToggleMotor          motor         3               
// ServoB               servo         B               
// ServoY               servo         A               
// ServoSlow            servo         C               
// ---- END VEXCODE CONFIGURED DEVICES ----
using namespace vex;

bool xToggle = 0;
bool bToggle = 0;
bool yToggle = 0;
int servoYPos = 0;
int servoBPos = 0;
int servoSlowPos = 0;
int bclose = 0;
int bopen = 100;
int yclose = 0;
int yopen = 100;

int main() {
  while (true) {
    int leftSpeed = Controller1.Axis3.position();
    int rightSpeed = Controller1.Axis2.position();
    LeftMotor.spin(forward, (abs(leftSpeed) >= 15) ? leftSpeed : 0, pct);
    RightMotor.spin(forward, (abs(rightSpeed) >= 15) ? rightSpeed : 0, pct);

    if (Controller1.ButtonX.pressing()) {
      xToggle = 1 - xToggle;
      while (Controller1.ButtonX.pressing()) { wait(10, msec); }
    }
    if (xToggle) ToggleMotor.spin(fwd, 100, pct);
    else ToggleMotor.stop();

    if (Controller1.ButtonB.pressing()) {
      bToggle = 1 - bToggle;
      while (Controller1.ButtonB.pressing()) { wait(10, msec); }
    }
    servoBPos = (bToggle) ? bopen : bclose;
    ServoB.setPosition(servoBPos, degrees);

    if (Controller1.ButtonY.pressing()) {
      yToggle = 1 - yToggle;
      while (Controller1.ButtonY.pressing()) { wait(10, msec); }
    }
    servoYPos = (yToggle) ? yopen : yclose;
    ServoY.setPosition(servoYPos, degrees);

    if (Controller1.ButtonR1.pressing()) servoSlowPos += 2;
    if (Controller1.ButtonR2.pressing()) servoSlowPos -= 2;
    ServoSlow.setPosition(servoSlowPos, degrees);
  }
}
