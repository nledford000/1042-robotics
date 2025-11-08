#include "vex.h"

// ---- START VEXCODE CONFIGURED DEVICES ----
// Robot Configuration:
// [Name]               [Type]        [Port(s)]
// Controller1          controller                    
// ArmMotor             motor         3               
// NeuralMotor          motor         4               
// LeftMotor            motor         1               
// RightMotor           motor         2               
// ServoOC              servo         B               
// ArmPot               pot           D               
// NeuralPot            pot           E               
// ServoInc             servo         G               
// ---- END VEXCODE CONFIGURED DEVICES ----

using namespace vex;

const int driveDeadband = 15;
const int driveSlewRate = 4;

enum ArmPos { ARM_LOW, ARM_MID, ARM_HIGH };
double armTargets[] = {223, 60, 1};
ArmPos armState = ARM_LOW;

const double armMin = 0;
const double armMax = 230;

enum NeuralPos { NP1, NP2, NP3, NP4 };
double neuralTargets[] = {0, 67, 151, 250};
NeuralPos neuralState = NP1;

bool armAutoMove = false;
bool neuralAutoMove = false;
double armTarget = armTargets[armState];
double neuralTarget = neuralTargets[neuralState];

double servoIncPos = 0;
const double servoIncStep = 10;
const double servoIncMin = 0;
const double servoIncMax = 180;

template <typename T>
T clamp(T value, T minV, T maxV) {
  return (value < minV) ? minV : (value > maxV) ? maxV : value;
}

bool magState = false;
void toggleMag() {
  magState = !magState;
  ServoOC.setPosition(magState ? 100 : 0, degrees);
}

void cycleArmPosition() {
  armState = static_cast<ArmPos>((armState + 1) % 3);
  armTarget = armTargets[armState];
  armAutoMove = true;
}

void cycleNeuralPosition() {
  neuralState = static_cast<NeuralPos>((neuralState + 1) % 4);
  neuralTarget = neuralTargets[neuralState];
  neuralAutoMove = true;
}

void incrementServo() {
  servoIncPos += servoIncStep;
  if (servoIncPos > servoIncMax) servoIncPos = servoIncMax;
  ServoInc.setPosition(servoIncPos, degrees);
}

void armManualUp() {
  armAutoMove = false;
  if (ArmPot.value(deg) > armMin)
    ArmMotor.spin(forward, 100, pct);
  else
    ArmMotor.stop(hold);
}

void armManualDown() {
  armAutoMove = false;
  if (ArmPot.value(deg) < armMax)
    ArmMotor.spin(reverse, 60, pct);
  else
    ArmMotor.stop(hold);
}

double kP_arm = 0.9, kD_arm = 0.12;
double prevErrArm = 0;

int armThread() {
  while(true) {
    if(armAutoMove) {
      double cur = ArmPot.value(deg);
      double err = armTarget - cur;
      if ((cur <= armMin && err > 0) || (cur >= armMax && err < 0)) {
        ArmMotor.stop(hold);
        this_thread::sleep_for(20);
        continue;
      }
      double der = err - prevErrArm;
      double power = clamp(kP_arm * err + kD_arm * der, -100.0, 100.0);
      ArmMotor.spin(power > 0 ? reverse : forward, fabs(power), pct);
      prevErrArm = err;
    } else if(!Controller1.ButtonR1.pressing() && !Controller1.ButtonR2.pressing()) {
      ArmMotor.stop(hold);
    }
    this_thread::sleep_for(20);
  }
  return 0;
}

double kP_neural = 0.8, kD_neural = 0.1;
double prevErrNeural = 0;

int neuralThread() {
  while(true) {
    if(neuralAutoMove) {
      double cur = NeuralPot.value(deg);
      double err = neuralTarget - cur;
      double der = err - prevErrNeural;
      double power = clamp(kP_neural * err + kD_neural * der, -100.0, 100.0);
      NeuralMotor.spin(power > 0 ? reverse : forward, fabs(power), pct);
      prevErrNeural = err;

      if (neuralState == NP4 && fabs(err) < 3) {
        this_thread::sleep_for(300);
        neuralState = NP1;
        neuralTarget = neuralTargets[neuralState];
      }

      /*
      if (neuralState == NP4 && fabs(err) < 3) {
        while (!BumpSwitch.pressing()) {
          NeuralMotor.spin(reverse, 40, pct);
        }
        NeuralMotor.stop(hold);
        neuralState = NP1;
        neuralTarget = neuralTargets[neuralState];
      }
      */
    } else {
      NeuralMotor.stop(hold);
    }
    this_thread::sleep_for(20);
  }
  return 0;
}

struct Action {
  bool last;
  void (*onPress)();
  bool (*isDown)();
  void update() {
    bool now = isDown();
    if(now && !last) onPress();
    last = now;
  }
};

int leftCmd = 0, rightCmd = 0;

void updateDrive() {
  int L = Controller1.Axis3.position();
  int R = Controller1.Axis2.position();
  if(abs(L) < driveDeadband) L = 0;
  if(abs(R) < driveDeadband) R = 0;
  leftCmd += (L > leftCmd ? driveSlewRate : (L < leftCmd ? -driveSlewRate : 0));
  rightCmd += (R > rightCmd ? driveSlewRate : (R < rightCmd ? -driveSlewRate : 0));
  LeftMotor.spin(forward, leftCmd, pct);
  RightMotor.spin(forward, rightCmd, pct);
}

// ---------------- NEW SET POSITION FUNCTIONS ----------------
void setArmPosition() {
  armTargets[armState] = ArmPot.value(deg);
  armTarget = armTargets[armState];
}

void setNeuralPosition() {
  neuralTargets[neuralState] = NeuralPot.value(deg);
  neuralTarget = neuralTargets[neuralState];
}
// ------------------------------------------------------------

int main() {
  vexcodeInit();
  thread t1(armThread);
  thread t2(neuralThread);
  Action actions[] = {
    {false, toggleMag, []{ return Controller1.ButtonY.pressing(); }},
    {false, cycleNeuralPosition, []{ return Controller1.ButtonX.pressing(); }},
    {false, cycleArmPosition, []{ return Controller1.ButtonB.pressing(); }},
    {false, armManualDown, []{ return Controller1.ButtonR2.pressing(); }},
    {false, armManualUp, []{ return Controller1.ButtonR1.pressing(); }},
    {false, incrementServo, []{ return Controller1.ButtonA.pressing(); }},
    {false, setArmPosition, []{ return Controller1.ButtonUp.pressing(); }},
    {false, setNeuralPosition, []{ return Controller1.ButtonDown.pressing(); }}
  };
  while(true) {
    updateDrive();
    for(auto &a : actions) a.update();
    this_thread::sleep_for(20);
  }
}
