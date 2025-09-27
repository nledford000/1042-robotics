/*----------------------------------------------------------------------------*/
/*                                                                            */
/*    Module:       main.cpp                                                  */
/*    Author:       C:\Users\student                                          */
/*    Created:      Tue Sep 16 2025                                           */
/*    Description:  V5 project                                                */
/*                                                                            */
/*----------------------------------------------------------------------------*/

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

#include "vex.h"

using namespace vex;

bool xToggle = 0;
bool bToggle = 0;
bool yToggle= 0;
int servoYPos = 0;
int servoBPos= 0;
int servoSlowPos = 0;
int bclose = 0;
int bopen = 100;
int yclose = 0;
int yopen = 100;

int main() {
	while (true) {
		// --- Tank Drive ---
		int leftSpeed = Controller1.Axis3.position();
		int rightSpeed = Controller1.Axis2.position();
		LeftMotor.spin(forward, (abs(leftSpeed) >= 15) ? leftSpeed : 0, pct);
		RightMotor.spin(forward, (abs(rightSpeed) >= 15) ? rightSpeed : 0, pct);

		// --- Button X Toggle ---
		if (Controller1.ButtonX.pressing()) {
			xToggle = 1 - xToggle;
			while (Controller1.ButtonX.pressing()) { wait(10, msec); }
		}
		if (xToggle == 1) {
			ToggleMotor.spin(fwd, 100, pct);
			} else {
			ToggleMotor.stop();
		}

		// --- Button B Toggle Servo ---
		if (Controller1.ButtonB.pressing()) {
			bToggle = 1 - bToggle;
			while (Controller1.ButtonB.pressing()) { wait(10, msec); }
		}
		servoBPos = (bToggle == 1) ? bopen : bclose;
		ServoB.setPosition(servoBPos, degrees);

		// --- Button Y Toggle Servo ---
		if (Controller1.ButtonY.pressing()) {
			yToggle = 1 - yToggle;
			while (Controller1.ButtonY.pressing()) { wait(10, msec); }
		}
		servoYPos = (yToggle == 1) ? yopen : yclose;
		ServoY.setPosition(servoYPos, degrees);

		// --- R1 & R2 Continuous Servo ---
		if (Controller1.ButtonR1.pressing()) {
			servoSlowPos++;
			wait(10, msec);
			}else if (Controller1.ButtonR2.pressing()) {
			servoSlowPos--;
			wait(10, msec);
			} else{
		}
		Brain.Screen.clearLine();
		Brain.Screen.print("%d", servoSlowPos);
		if (servoSlowPos > 45) servoSlowPos = 45;
		if (servoSlowPos < -45) servoSlowPos = -45;

		ServoSlow.setPosition(servoSlowPos, degrees);
	}
}

