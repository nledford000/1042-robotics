// main_0.1.1.cpp
// Add deadzone filtering

#include "vex.h"

// ---- START VEXCODE CONFIGURED DEVICES ----
// Robot Configuration:
// [Name]               [Type]        [Port(s)]
// Controller1          controller                    
// LeftMotor            motor         1               
// RightMotor           motor         2               
// ---- END VEXCODE CONFIGURED DEVICES ----
using namespace vex;

static int applyDeadzone(int v) {
  return (abs(v) > 5) ? v : 0;
}

int main() {
  while (true) {
    int leftSpeed = applyDeadzone(Controller1.Axis3.position());
    int rightSpeed = applyDeadzone(Controller1.Axis2.position());
    LeftMotor.spin(forward, leftSpeed, pct);
    RightMotor.spin(forward, rightSpeed, pct);
  }
}
