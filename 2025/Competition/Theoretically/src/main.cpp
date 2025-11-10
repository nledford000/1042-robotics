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
#include <chrono> // for std::chrono::steady_clock::time_point, milliseconds, duration_cast

using namespace vex;

const int driveDeadband = 15;
const int driveSlewRate = 4;

enum Mode { MANUAL, AUTO, SETUP };
Mode currentMode = MANUAL;

enum ArmPos { ARM_LOW, ARM_MID, ARM_HIGH };
double armTargets[] = {223, 60, 1};
ArmPos armState = ARM_LOW;

double armMin = 0;
double armMax = 230;

enum NeuralPos { NP1, NP2, NP3, NP4 };
double neuralTargets[] = {0, 67, 151, 250};
NeuralPos neuralState = NP1;

double neuralMin = 0;
double neuralMax = 250;

bool armAutoMove = false;
bool neuralAutoMove = false;
double armTarget = armTargets[armState];
double neuralTarget = neuralTargets[neuralState];

double servoIncPos = 0;
const double servoIncStep = 10;
const double servoIncMax = 180;

bool magState = false;

template <typename T>
T clamp(T value, T minV, T maxV) {
  return (value < minV) ? minV : (value > maxV) ? maxV : value;
}

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

double kP_arm = 0.9, kD_arm = 0.12;
double prevErrArm = 0;

double kP_neural = 0.8, kD_neural = 0.1;
double prevErrNeural = 0;

double softLimitScale(double value, double minLimit, double maxLimit) {
  double range = maxLimit - minLimit;
  double margin = 20.0;
  if (value < minLimit + margin) return clamp((value - minLimit) / margin, 0.0, 1.0);
  if (value > maxLimit - margin) return clamp((maxLimit - value) / margin, 0.0, 1.0);
  return 1.0;
}

int armThread() {
  while(true) {
    if (armAutoMove) {
      double cur = ArmPot.value(deg);
      double err = armTarget - cur;
      double der = err - prevErrArm;
      double scale = softLimitScale(cur, armMin, armMax);
      double power = clamp((kP_arm * err + kD_arm * der) * scale, -100.0, 100.0);
      ArmMotor.spin(power > 0 ? reverse : forward, fabs(power), pct);
      prevErrArm = err;
    } else if(!Controller1.ButtonR1.pressing() && !Controller1.ButtonR2.pressing()) {
      ArmMotor.stop(hold);
    }
    this_thread::sleep_for(20);
  }
  return 0;
}

int neuralThread() {
  while(true) {
    if (neuralAutoMove) {
      double cur = NeuralPot.value(deg);
      double err = neuralTarget - cur;
      double der = err - prevErrNeural;
      double scale = softLimitScale(cur, neuralMin, neuralMax);
      double power = clamp((kP_neural * err + kD_neural * der) * scale, -100.0, 100.0);
      NeuralMotor.spin(power > 0 ? reverse : forward, fabs(power), pct);
      prevErrNeural = err;

      if (neuralState == NP4 && fabs(err) < 3) {
        this_thread::sleep_for(300);
        neuralState = NP1;
        neuralTarget = neuralTargets[neuralState];
      }
    } else {
      NeuralMotor.stop(hold);
    }
    this_thread::sleep_for(20);
  }
  return 0;
}

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

bool doubleTapUp = false;
bool doubleTapDown = false;

// fully qualified chrono types and initialize to now
std::chrono::steady_clock::time_point lastUpPress = std::chrono::steady_clock::now();
std::chrono::steady_clock::time_point lastDownPress = std::chrono::steady_clock::now();

void checkDoubleTap() {
  auto now = std::chrono::steady_clock::now();
  if (Controller1.ButtonUp.pressing()) {
    if (std::chrono::duration_cast<std::chrono::milliseconds>(now - lastUpPress).count() < 300) {
      doubleTapUp = true;
      currentMode = SETUP;
    }
    lastUpPress = now;
  }
  if (Controller1.ButtonDown.pressing()) {
    if (std::chrono::duration_cast<std::chrono::milliseconds>(now - lastDownPress).count() < 300) {
      doubleTapDown = true;
    }
    lastDownPress = now;
  }
}

void setupScreen(const char* motor, int posSlot) {
  Brain.Screen.clearScreen();
  Brain.Screen.setCursor(1,1);
  Brain.Screen.print("MODE: SETUP");
  Brain.Screen.newLine();
  Brain.Screen.print("Motor: %s", motor);
  Brain.Screen.newLine();
  Brain.Screen.print("Position Slot: %d", posSlot+1);
}

void flashSave() {
  Brain.Screen.setFillColor(red);
  Brain.Screen.drawRectangle(0,0,480,240);
  this_thread::sleep_for(150);
  Brain.Screen.setFillColor(black);
  Brain.Screen.clearScreen();
}

void setArmPosition(int slot) {
  // slot is validated by caller now (0..2)
  armTargets[slot] = ArmPot.value(deg);
  armTarget = armTargets[slot];
  flashSave();
}

void setNeuralPosition(int slot) {
  neuralTargets[slot] = NeuralPot.value(deg);
  neuralTarget = neuralTargets[slot];
  flashSave();
}

int main() {
  vexcodeInit();
  thread t1(armThread);
  thread t2(neuralThread);

  int setupMotor = 0; 
  int setupSlot = 0;

  while(true) {
    updateDrive();
    checkDoubleTap();

    if (currentMode == SETUP) {
      const char* motorName = setupMotor == 0 ? "ARM" : "NEURAL";
      setupScreen(motorName, setupSlot);

      if (Controller1.ButtonRight.pressing()) setupMotor = (setupMotor + 1) % 2;

      // wrap setupSlot depending on selected motor to avoid out-of-bounds
      if (Controller1.ButtonLeft.pressing()) {
        if (setupMotor == 0) setupSlot = (setupSlot + 1) % 3; // ARM has 3 slots
        else setupSlot = (setupSlot + 1) % 4; // NEURAL has 4 slots
      }

      if (Controller1.ButtonY.pressing()) {
        if (setupMotor == 0) setArmPosition(setupSlot);
        else setNeuralPosition(setupSlot);
      }
      if (Controller1.ButtonB.pressing()) currentMode = MANUAL;
    }

    if (doubleTapDown) {
      ArmMotor.spin(reverse, 40, pct);
      NeuralMotor.spin(reverse, 40, pct);
      this_thread::sleep_for(2000);
      ArmMotor.stop(hold);
      NeuralMotor.stop(hold);
      armMin = ArmPot.value(deg);
      neuralMin = NeuralPot.value(deg);
      doubleTapDown = false;
    }

    Brain.Screen.setCursor(6,1);
    const char* modeName = currentMode == MANUAL ? "MANUAL" : currentMode == AUTO ? "AUTO" : "SETUP";
    Brain.Screen.print("Mode: %s", modeName);

    this_thread::sleep_for(20);
  }
}
