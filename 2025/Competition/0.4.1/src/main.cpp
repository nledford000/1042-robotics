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

bool bToggle = false;
bool yToggle = false;
bool xToggle = false;
int servoSlowPos = 90;

int main() {
  while(true) {
    int leftSpeed  = Controller1.Axis3.position();
    int rightSpeed = Controller1.Axis2.position();
    if (abs(leftSpeed) < 15) leftSpeed = 0;
    if (abs(rightSpeed) < 15) rightSpeed = 0;
    LeftMotor.spin(forward, leftSpeed, pct);
    RightMotor.spin(forward, rightSpeed, pct);


		if (Controller1.ButtonX.pressing()) {
			xToggle = !xToggle;
			while (Controller1.ButtonX.pressing()) wait(10, msec);
		}
		if (xToggle){
			ToggleMotor.spin(forward, 100, pct);
		}
		else{
			ToggleMotor.stop();
		}

    if (Controller1.ButtonB.pressing()) {
      bToggle = !bToggle;
      while (Controller1.ButtonB.pressing()) wait(10, msec);
    }
    ServoB.setPosition(bToggle ? 100 : 0, degrees);

    if (Controller1.ButtonY.pressing()) {
      yToggle = !yToggle;
      while (Controller1.ButtonY.pressing()) wait(10, msec);
    }
    ServoY.setPosition(yToggle ? 100 : 0, degrees);

    if (Controller1.ButtonR1.pressing()) servoSlowPos += 2;
    if (Controller1.ButtonR2.pressing()) servoSlowPos -= 2;
    if (servoSlowPos > 180) servoSlowPos = 180;
    if (servoSlowPos < 0)   servoSlowPos = 0;
    ServoSlow.setPosition(servoSlowPos, degrees);
  }
}
