#include "vex.h"

// ---- START VEXCODE CONFIGURED DEVICES ----
// Robot Configuration:
// [Name]               [Type]        [Port(s)]
// LeftMotor            motor         1               
// RightMotor           motor         2               
// ArmMotor             motor         3               
// ServoOC              servo         A               
// ServoInc             servo         C               
// ArmPot               pot           E               
// NeuralPot            pot           G               
// NeuralMotor          motor         4               
// Controller1          controller                    
// ---- END VEXCODE CONFIGURED DEVICES ----

#include <cmath>
using namespace vex;

const int driveDeadband = 15;

const double armLow = 180;
const double armMid = 200;
const double armHigh = 230;//180, 200, 228
const double armUpSpeed = 100;
const double armDownSpeed = 100;
const double armTolerance = 4;

const int magOpen = 100;
const int magClosed = 0;

const double neuralPos1 = 194;//194, 221, 250,  
const double neuralPos2 = 221;
const double neuralPos3 = 250;

const double neuralUpSpeed = 60;
const double neuralDownSpeed = 60;
const double neuralTolerance = 2;

const double gravK = 15.0;
const double angleOffset = 204.0; // set this to your arm's horizontal potentiometer reading
//MotorPower=BaseSpeed+-(gravK x |cos(angle)|)

int armState = 1;
double armTarget = armLow;
bool armAutoMove = false;
bool magToggle = false;
int neuralState = 1;
double neuralTarget = neuralPos1;
bool neuralAutoMove = false;

double servoIncValue = 15;
double servoIncPos = -25; 
double servoInit = -25;
double servoMax = 70;
//double servoIncPos = -60; 

bool autonomousMode = false;


void toggleMagAction() {
  magToggle = !magToggle;
  ServoOC.setPosition(magToggle ? magOpen : magClosed, degrees);
}

void cycleArmPosition() {
  armState++;
  if (armState > 3) armState = 1;
  if (armState == 1) armTarget = armLow;
  else if (armState == 2) armTarget = armMid;
  else if (armState == 3) armTarget = armHigh;
  armAutoMove = true;
}

void cycleNeuralPosition() {
  neuralState++;
  if (neuralState > 4) neuralState = 1;
  if (neuralState == 1) neuralTarget = neuralPos1;
  else if (neuralState == 2) neuralTarget = neuralPos2;
  else if (neuralState == 3) neuralTarget = neuralPos3;
  neuralAutoMove = true;
}

double clampPct(double v) {
  if (v > 100.0) return 100.0;
  if (v < -100.0) return -100.0;
  return v;
}

double getGravityAssistPct() {
  const double PI = 3.1415926535896;
  double angleDeg = ArmPot.value(deg);
  double angleRad = (angleDeg - angleOffset) * PI / 180.0;
  return gravK * fabs(cos(angleRad));
}

void armUpManual() {
  double grav = getGravityAssistPct();
  double out = clampPct(armUpSpeed + grav);
  ArmMotor.spin(forward, out, pct);
  armAutoMove = false;
}

void armDownManual() {
  double grav = getGravityAssistPct();
  double out = clampPct(armDownSpeed - grav);
  if (out < 10) out = 10;
  ArmMotor.spin(reverse, out, pct);
  armAutoMove = false;
}
void cycleServoInc() {
    servoIncPos += servoIncValue;
    if (servoIncPos > servoMax) {
        servoIncPos = servoInit;
    }
    ServoInc.setPosition(servoIncPos, degrees);
}

bool buttonYPressed() { return Controller1.ButtonY.pressing(); }
bool buttonXPressed() { return Controller1.ButtonX.pressing(); }
bool buttonBPressed() { return Controller1.ButtonB.pressing(); }
bool buttonR1Pressed() { return Controller1.ButtonR1.pressing(); }
bool buttonR2Pressed() { return Controller1.ButtonR2.pressing(); }
bool buttonAPressed() { return Controller1.ButtonA.pressing(); }
bool buttonUpPressed() { return Controller1.ButtonUp.pressing(); }


struct Action {
  bool lastState;
  void (*onPress)();
  bool (*isPressed)();
  void update() {
    bool now = isPressed();
    if (now && !lastState && onPress)
      onPress();
    lastState = now;
  }
};

int armAutoThread() {
  while (true) {
    if (armAutoMove) {
      double current = ArmPot.value(deg);
      double error = armTarget - current;
      if (fabs(error) > armTolerance) {
        bool needUp = (error > 0);
        double grav = getGravityAssistPct();
        double base = needUp ? armUpSpeed : armDownSpeed;
        double out = needUp ? clampPct(base + grav) : clampPct(base - grav);
        if (!needUp && out < 10) out = 10;
        if (needUp) ArmMotor.spin(forward, out, pct);
        else ArmMotor.spin(reverse, out, pct);
      } else {
        ArmMotor.stop(hold);
        armAutoMove = false;
      }
    } else if (!Controller1.ButtonR1.pressing() && !Controller1.ButtonR2.pressing()) {
      ArmMotor.stop(hold);
    }
    this_thread::sleep_for(20);
  }
  return 0;
}

int neuralAutoThread() {
  while (true) {
    if (neuralAutoMove) {
      double current = NeuralPot.value(deg);
      double error = neuralTarget - current;
      if (fabs(error) > neuralTolerance) {
        NeuralMotor.spin(error > 0 ? reverse : forward,
                         (error > 0 ? neuralDownSpeed : neuralUpSpeed), pct);
      } else {
        NeuralMotor.stop(hold);
        neuralAutoMove = false;
      }
    } else {
      NeuralMotor.stop(hold);
    }
    this_thread::sleep_for(20);
  }
  return 0;
}

void macroSequence() {
    autonomousMode = true;

    magToggle = true;
    ServoOC.setPosition(magOpen, degrees);
    wait(500, msec);

    armTarget = armHigh;
    armAutoMove = true;
    int timeout = 3000;
    while (armAutoMove && timeout > 0) {
        this_thread::sleep_for(1);
        timeout -= 20;
    }

    magToggle = true;
    ServoOC.setPosition(magClosed, degrees);
    wait(500, msec);

    armTarget = armLow;
    armAutoMove = true;
    timeout = 3000;
    while (armAutoMove && timeout > 0) {
        this_thread::sleep_for(1);
        timeout -= 20;
    }

    // Step 5: Servo increment
    servoIncPos -= servoIncValue;
    if (servoIncPos < 0) servoIncPos = 180;
    ServoInc.setPosition(servoIncPos, degrees);
    wait(500, msec);

    autonomousMode = false;
}


int main() {
  vexcodeInit();
  thread armThread(armAutoThread);
  thread neuralThread(neuralAutoThread);

  Action actions[] = {
    {false, toggleMagAction, buttonYPressed},
    {false, cycleNeuralPosition, buttonXPressed},
    {false, cycleArmPosition, buttonBPressed},
    {false, armDownManual, buttonR2Pressed},
    {false, armUpManual, buttonR1Pressed},
    {false, cycleServoInc, buttonAPressed},
    {false, macroSequence, buttonUpPressed}
  };
  ServoInc.setPosition(servoInit, degrees);
  while (true) {
    int leftSpeed = Controller1.Axis3.position();
    int rightSpeed = Controller1.Axis2.position();
    if (abs(leftSpeed) < driveDeadband) leftSpeed = 0;
    if (abs(rightSpeed) < driveDeadband) rightSpeed = 0;
    LeftMotor.spin(forward, leftSpeed, pct);
    RightMotor.spin(forward, rightSpeed, pct);

    for (int i = 0; i < sizeof(actions)/sizeof(actions[0]); i++)
      actions[i].update();

    this_thread::sleep_for(20);
  }
}
