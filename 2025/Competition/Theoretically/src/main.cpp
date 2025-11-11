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

const int LOOP_MS = 20;
const double DT = LOOP_MS / 1000.0;

const int DRIVE_DEADBAND = 15;
const int DRIVE_SLEW = 4;

const double ARM_DEFAULT_MIN = 0.0;
const double ARM_DEFAULT_MAX = 230.0;
const double NEURAL_DEFAULT_MIN = 0.0;
const double NEURAL_DEFAULT_MAX = 250.0;

const double SERVO_INC_STEP = 10.0;
const double SERVO_INC_MAX = 180.0;

// PID defaults (tweak)
const double ARM_kP = 0.9;
const double ARM_kI = 0.03;
const double ARM_kD = 0.12;
const double NEURAL_kP = 0.8;
const double NEURAL_kI = 0.02;
const double NEURAL_kD = 0.10;

// limits
const double INT_CLAMP = 200.0;

// softLimitScale
const double SOFT_MARGIN = 20.0;

// tolerance(deg)
const double ARM_TOL = 2.0;
const double NEURAL_TOL = 2.5;

// motion time(sec)
const double MOVE_TIMEOUT = 5.0;

// Volatile for threa safe
volatile bool armAutoMove = false;
volatile bool neuralAutoMove = false;

volatile double armTarget = 0.0;
volatile double neuralTarget = 0.0;

volatile bool magState = false;

double armMin = ARM_DEFAULT_MIN;
double armMax = ARM_DEFAULT_MAX;
double neuralMin = NEURAL_DEFAULT_MIN;
double neuralMax = NEURAL_DEFAULT_MAX;

// presets (modifiable in SETUP) 
double armPresets[3] = {223.0, 60.0, 1.0};
double neuralPresets[4] = {0.0, 67.0, 151.0, 250.0};

int armPresetIndex = 0;
int neuralPresetIndex = 0;

double servoIncPos = 0.0;

int leftCmd = 0, rightCmd = 0;

enum Mode { MANUAL, AUTO, SETUP };
volatile Mode currentMode = MANUAL;

// button edge state ):
bool prevUp = false, prevDown = false, prevLeft = false, prevRight = false;
bool prevA = false, prevB = false, prevX = false, prevY = false;
bool prevR1 = false, prevR2 = false, prevL1 = false, prevL2 = false;

unsigned long ticks = 0;
unsigned long lastUpTick = 0;
unsigned long lastDownTick = 0;
const unsigned long DOUBLE_TAP_TICKS = 300 / LOOP_MS; // 300ms window

//(legit copied from library)
template<typename T>
T clampT(T v, T lo, T hi) {
  return (v < lo) ? lo : (v > hi) ? hi : v;
}

double softLimitScale(double value, double minLimit, double maxLimit) {
  double range = maxLimit - minLimit;
  if (range <= 0.0) return 0.0;
  if (value < minLimit + SOFT_MARGIN) return clampT((value - minLimit) / SOFT_MARGIN, 0.0, 1.0);
  if (value > maxLimit - SOFT_MARGIN) return clampT((maxLimit - value) / SOFT_MARGIN, 0.0, 1.0);
  return 1.0;
}

void toggleMag() {
  magState = !magState;
  ServoOC.setPosition(magState ? 100 : 0, degrees);
}

//no std::chrono
// actuator threads use fixed dt btw
int armThread() {
  double integral = 0.0;
  double prevErr = 0.0;
  double prevPos = ArmPot.value(deg);
  int moveTickCounter = 0; //count when move started
  bool moving = false;

  while (true) {
    // sample
    double cur = ArmPot.value(deg);
    double tgt = armTarget; // volatile
    double err = tgt - cur;

    if (armAutoMove) {
      if (!moving) {
        moving = true;
        moveTickCounter = 0;
        integral = 0.0;
        prevErr = err;
        prevPos = cur;
      }

      // PID
      integral += err * DT;
      // no windup
      integral = clampT(integral, -INT_CLAMP, INT_CLAMP);
      double derivative = (err - prevErr) / DT;
      prevErr = err;

      double raw = ARM_kP * err + ARM_kI * integral + ARM_kD * derivative;
      double scale = softLimitScale(cur, armMin, armMax);
      raw *= scale;

      // simple ramp: propo to error magnitude (no full power jumps)
      double ramp = clampT(fabs(err) / 50.0, 0.1, 1.0);
      double commanded = raw * ramp;
      commanded = clampT(commanded, -100.0, 100.0);

      if (fabs(err) <= ARM_TOL) {
        ArmMotor.stop(hold);
        armAutoMove = false;
        moving = false;
        integral = 0.0;
      } else {
        // timout
        moveTickCounter++;
        double elapsedSec = (moveTickCounter * LOOP_MS) / 1000.0;
        if (elapsedSec > MOVE_TIMEOUT) {
          ArmMotor.stop(hold);
          armAutoMove = false;
          moving = false;
          integral = 0.0;
        } else {
          ArmMotor.spin((commanded > 0.0) ? reverse : forward, fabs(commanded), pct);
        }
      }
    } else {
      // not auto move: if no manual input, hold position
      ArmMotor.stop(hold);
      moving = false;
      integral = 0.0;
      prevErr = 0.0;
    }

    this_thread::sleep_for(LOOP_MS);
  }
  return 0;
}

int neuralThread() {
  double integral = 0.0;
  double prevErr = 0.0;
  double prevPos = NeuralPot.value(deg);
  int moveTickCounter = 0;
  bool moving = false;

  while (true) {
    double cur = NeuralPot.value(deg);
    double tgt = neuralTarget;
    double err = tgt - cur;

    if (neuralAutoMove) {
      if (!moving) {
        moving = true;
        moveTickCounter = 0;
        integral = 0.0;
        prevErr = err;
        prevPos = cur;
      }

      integral += err * DT;
      integral = clampT(integral, -INT_CLAMP, INT_CLAMP);
      double derivative = (err - prevErr) / DT;
      prevErr = err;

      double raw = NEURAL_kP * err + NEURAL_kI * integral + NEURAL_kD * derivative;
      double scale = softLimitScale(cur, neuralMin, neuralMax);
      raw *= scale;

      double ramp = clampT(fabs(err) / 50.0, 0.1, 1.0);
      double commanded = raw * ramp;
      commanded = clampT(commanded, -100.0, 100.0);

      if (fabs(err) <= NEURAL_TOL) {
        NeuralMotor.stop(hold);
        neuralAutoMove = false;
        moving = false;
        integral = 0.0;
      } else {
        moveTickCounter++;
        double elapsedSec = (moveTickCounter * LOOP_MS) / 1000.0;
        if (elapsedSec > MOVE_TIMEOUT) {
          NeuralMotor.stop(hold);
          neuralAutoMove = false;
          moving = false;
          integral = 0.0;
        } else {
          NeuralMotor.spin((commanded > 0.0) ? reverse : forward, fabs(commanded), pct);
        }
      }
    } else {
      NeuralMotor.stop(hold);
      moving = false;
      integral = 0.0;
      prevErr = 0.0;
    }

    this_thread::sleep_for(LOOP_MS);
  }
  return 0;
}

void updateDrive() {
  int L = Controller1.Axis3.position();
  int R = Controller1.Axis2.position();
  if (abs(L) < DRIVE_DEADBAND) L = 0;
  if (abs(R) < DRIVE_DEADBAND) R = 0;

  leftCmd += (L > leftCmd ? DRIVE_SLEW : (L < leftCmd ? -DRIVE_SLEW : 0));
  rightCmd += (R > rightCmd ? DRIVE_SLEW : (R < rightCmd ? -DRIVE_SLEW : 0));

  LeftMotor.spin(forward, leftCmd, pct);
  RightMotor.spin(forward, rightCmd, pct);
}

bool rose(bool cur, bool &prev) {
  bool r = cur && !prev;
  prev = cur;
  return r;
}

void cycleArmPreset() {
  armPresetIndex = (armPresetIndex + 1) % 3;
  armTarget = armPresets[armPresetIndex];
  armAutoMove = true;
}

void cycleNeuralPreset() {
  neuralPresetIndex = (neuralPresetIndex + 1) % 4;
  neuralTarget = neuralPresets[neuralPresetIndex];
  neuralAutoMove = true;
}

void setArmPresetSlot(int slot) {
  if (slot < 0 || slot > 2) return;
  armPresets[slot] = ArmPot.value(deg);
  armTarget = armPresets[slot];

  Brain.Screen.setFillColor(red);
  Brain.Screen.drawRectangle(0,0,480,240);
  this_thread::sleep_for(120);
  Brain.Screen.setFillColor(black);
  Brain.Screen.clearScreen();
}

void setNeuralPresetSlot(int slot) {
  if (slot < 0 || slot > 3) return;
  neuralPresets[slot] = NeuralPot.value(deg);
  neuralTarget = neuralPresets[slot];
  Brain.Screen.setFillColor(red);
  Brain.Screen.drawRectangle(0,0,480,240);
  this_thread::sleep_for(120);
  Brain.Screen.setFillColor(black);
  Brain.Screen.clearScreen();
}

void calibrateMins() {
  const int pow = 30;
  const int ms = 1200;
  ArmMotor.spin(reverse, pow, pct);
  NeuralMotor.spin(reverse, pow, pct);
  this_thread::sleep_for(ms);
  ArmMotor.stop(hold);
  NeuralMotor.stop(hold);
  armMin = ArmPot.value(deg);
  neuralMin = NeuralPot.value(deg);
}

void incrementServo() {
  servoIncPos += SERVO_INC_STEP;
  if (servoIncPos > SERVO_INC_MAX) servoIncPos = SERVO_INC_MAX;
  ServoInc.setPosition(servoIncPos, degrees);
}

int main() {
  vexcodeInit();

  armTarget = armPresets[armPresetIndex];
  neuralTarget = neuralPresets[neuralPresetIndex];

  thread tArm(armThread);
  thread tNeural(neuralThread);

  unsigned long lastScreenTick = 0;
  const unsigned long SCREEN_TICKS = 250 / LOOP_MS; 

  while (true) {
    ticks++;
    updateDrive();

    if (rose(Controller1.ButtonUp.pressing(), prevUp)) {
      if ((ticks - lastUpTick) <= DOUBLE_TAP_TICKS) {
        currentMode = SETUP;
      }
      lastUpTick = ticks;
    }

    if (rose(Controller1.ButtonDown.pressing(), prevDown)) {
      if ((ticks - lastDownTick) <= DOUBLE_TAP_TICKS) {
        calibrateMins();
      }
      lastDownTick = ticks;
    }

    if (Controller1.ButtonR1.pressing() || Controller1.ButtonR2.pressing()) {
      armAutoMove = false;
      if (Controller1.ButtonR1.pressing()) {
        ArmMotor.spin(reverse, 40, pct);
      } else if (Controller1.ButtonR2.pressing()) {
        ArmMotor.spin(forward, 40, pct);
      }
    }

    if (Controller1.ButtonL1.pressing() || Controller1.ButtonL2.pressing()) {
      neuralAutoMove = false;
      if (Controller1.ButtonL1.pressing()) {
        NeuralMotor.spin(reverse, 40, pct);
      } else if (Controller1.ButtonL2.pressing()) {
        NeuralMotor.spin(forward, 40, pct);
      }
    }

    // cycles: A >> arm, X >> neural
    if (rose(Controller1.ButtonA.pressing(), prevA)) {
      cycleArmPreset();
    }
    if (rose(Controller1.ButtonX.pressing(), prevX)) {
      cycleNeuralPreset();
    }

    if (rose(Controller1.ButtonY.pressing(), prevY)) {
      toggleMag();
    }

    if (rose(Controller1.ButtonB.pressing(), prevB)) {
      incrementServo();
    }

    static int setupMotor = 0; //0 arm, 1 neural
    static int setupSlot = 0;
    if (currentMode == SETUP) {
      Brain.Screen.clearScreen();
      Brain.Screen.setCursor(1,1);
      Brain.Screen.print("MODE: SETUP");
      Brain.Screen.newLine();
      Brain.Screen.print("Motor: %s", (setupMotor == 0) ? "ARM" : "NEURAL");
      Brain.Screen.newLine();
      Brain.Screen.print("Slot: %d", setupSlot + 1);
      Brain.Screen.newLine();
      Brain.Screen.print("Right: toggle motor, Left: next slot");
      Brain.Screen.newLine();
      Brain.Screen.print("Y: save current pos, B: exit");

      if (rose(Controller1.ButtonRight.pressing(), prevRight)) {
        setupMotor = (setupMotor + 1) % 2;
      }

      //cycles slot
      if (rose(Controller1.ButtonLeft.pressing(), prevLeft)) {
        if (setupMotor == 0) setupSlot = (setupSlot + 1) % 3;
        else setupSlot = (setupSlot + 1) % 4;
      }

      if (rose(Controller1.ButtonY.pressing(), prevY)) {
        if (setupMotor == 0) setArmPresetSlot(setupSlot);
        else setNeuralPresetSlot(setupSlot);
      }

      if (rose(Controller1.ButtonB.pressing(), prevB)) {
        currentMode = MANUAL;
        Brain.Screen.clearScreen();
      }
    }

    // autowrap NP4>>NP1: if neural preset index is 3 and reached recently
    if (!neuralAutoMove && neuralPresetIndex == 3) {
      double cur = NeuralPot.value(deg);
      double tgt = neuralPresets[3];
      if (fabs(cur - tgt) < 3.0) {
        this_thread::sleep_for(300);
        neuralPresetIndex = 0;
        neuralTarget = neuralPresets[0];
        neuralAutoMove = true;
      }
    }

    //telemetry if low rate
    if ((ticks - lastScreenTick) >= SCREEN_TICKS) {
      // oh my bruh this took to long
      lastScreenTick = ticks;
      Brain.Screen.clearScreen();
      Brain.Screen.setCursor(1,1);
      const char* modeStr = (currentMode == MANUAL) ? "MANUAL" : (currentMode == AUTO) ? "AUTO" : "SETUP";
      Brain.Screen.print("Mode: %s", modeStr);
      Brain.Screen.newLine();
      Brain.Screen.print("Arm cur: %.1f tgt: %.1f auto:%d", ArmPot.value(deg), armTarget, armAutoMove ? 1 : 0);
      Brain.Screen.newLine();
      Brain.Screen.print("Neur cur: %.1f tgt: %.1f auto:%d", NeuralPot.value(deg), neuralTarget, neuralAutoMove ? 1 : 0);
      Brain.Screen.newLine();
      Brain.Screen.print("Mag:%d ServoInc:%.1f", magState ? 1 : 0, servoIncPos);
    }

    this_thread::sleep_for(LOOP_MS);
  }

  return 0;
}
