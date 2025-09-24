#include "vex.h"

// ---- START VEXCODE CONFIGURED DEVICES ----
// Robot Configuration:
// [Name]               [Type]        [Port(s)]
// Controller1          controller                    
// LeftMotor            motor         1               
// RightMotor           motor         2               
// ---- END VEXCODE CONFIGURED DEVICES ----

using namespace vex;
int main() {
	vexcodeInit();
	while(true) {
		int l = Controller1.Axis3.position();
		int r = Controller1.Axis2.position();
		LeftMotor.spin(fwd, l, pct);
		RightMotor.spin(fwd, r, pct);
	}
}
