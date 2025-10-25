#include "vex.h"

// ---- START VEXCODE CONFIGURED DEVICES ----
// Robot Configuration:
// [Name]               [Type]        [Port(s)]
// Controller1          controller                    
// ArmMotor             motor         3               
// NeuralMotor          motor         4               
// NeuralPot            pot           A               
// ServoOC              servo         B               
// ServoPush            servo         C               
// LeftMotor            motor         1               
// RightMotor           motor         2               
// ArmPot               pot           D               
// ---- END VEXCODE CONFIGURED DEVICES ----

using namespace vex;

// Constants
const int driveDeadband = 15;
const int magOpen = 100;
const int magClosed = 0;
const int panelForward = 80;
const int panelBack = 0;

const double armLow = 225;
const double armMid = 125;
const double armHigh = 75;

const double armSpeed = 60;
const double armTolerance = 15;

// State variables
int armState = 0;
double armTarget = 250;
bool armAutoMove = false;
bool magToggle = false;
bool panelToggle = false;
bool neuralToggle = false;

// Utility
double clamp(double value, double minVal, double maxVal) {
return value < minVal ? minVal : (value > maxVal ? maxVal : value);
}

// Arm control thread
int armControlThread() {
while (true) {
if (Controller1.ButtonR2.pressing()) {
ArmMotor.spin(forward, armSpeed, pct);
armAutoMove = false;
} else if (Controller1.ButtonR1.pressing()) {
ArmMotor.spin(reverse, armSpeed, pct);
armAutoMove = false;
} else if (armAutoMove) {
double current = ArmPot.angle(degrees);
if (fabs(armTarget - current) > armTolerance) {
ArmMotor.spin(current < armTarget ? forward : reverse, armSpeed, pct);
} else {
ArmMotor.stop(hold);
armAutoMove = false;
}
} else {
ArmMotor.stop(hold);
}
wait(20, msec);
}
return 0;
}

// Actions
void toggleMagAction() {
magToggle = !magToggle;
ServoOC.setPosition(magToggle ? magOpen : magClosed, degrees);
}

void togglePanelAction() {
panelToggle = !panelToggle;
ServoPush.setPosition(panelToggle ? panelForward : panelBack, degrees);
}

void cycleArmPosition() {
armState = (armState + 1) % 3;
armTarget = (armState == 0) ? armLow : (armState == 1) ? armMid : armHigh;
armAutoMove = true;
}

void toggleNeuralMotor() {
neuralToggle = !neuralToggle;
if (neuralToggle) {
NeuralMotor.spin(forward, 100, pct);
} else {
NeuralMotor.stop(coast);
}
}

// Button checks
bool buttonYPressed() { return Controller1.ButtonY.pressing(); }
bool buttonAPressed() { return Controller1.ButtonA.pressing(); }
bool buttonXPressed() { return Controller1.ButtonX.pressing(); }
bool buttonBPressed() { return Controller1.ButtonB.pressing(); }

// Action handler
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

int main() {
vexcodeInit();
thread armThread(armControlThread);

Action actions[] = {
{false, toggleMagAction, buttonYPressed},
{false, togglePanelAction, buttonAPressed},
{false, toggleNeuralMotor, buttonXPressed},
{false, cycleArmPosition, buttonBPressed}
};

while (true) {
int leftSpeed = Controller1.Axis3.position();
int rightSpeed = Controller1.Axis2.position();

if (abs(leftSpeed) < driveDeadband) leftSpeed = 0;
if (abs(rightSpeed) < driveDeadband) rightSpeed = 0;

LeftMotor.spin(forward, leftSpeed, pct);
RightMotor.spin(forward, rightSpeed, pct);

for (int i = 0; i < 4; i++) actions[i].update();

wait(20, msec);
}
}

