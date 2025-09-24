#include "vex.h"

// ---- START VEXCODE CONFIGURED DEVICES ----
// Robot Configuration:
// [Name]               [Type]        [Port(s)]
// Controller1          controller                    
// ToggleMotor          motor         3               
// LeftMotor            motor         1               
// RightMotor           motor         2               
// ---- END VEXCODE CONFIGURED DEVICES ----
using namespace vex;

bool xToggle = false;

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
	}
}
