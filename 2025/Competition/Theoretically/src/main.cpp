#include "vex.h"
using namespace vex;

// ---------------------------------------------------------
// GLOBALS
// ---------------------------------------------------------
vex::brain Brain;
vex::controller Controller1;
vex::motor ArmMotor = vex::motor(vex::PORT1);
vex::motor NeuralMotor = vex::motor(vex::PORT2);

// Potentiometer in RAW 12-bit mode (0–4095)
vex::pot ArmPot = vex::pot(Brain.ThreeWirePort.A);

bool magOn = false;
bool armAutoMove = false;
bool neuralAutoMove = false;

// Target raw positions for the arm (0–4095)
int armTargets[] = {8, 17, 25};
int currentArm = 0;

// Target raw positions for neural
int neuralTargets[] = {100, 500, 900};
int currentNeural = 0;


// ---------------------------------------------------------
// ACTION SYSTEM (USING REAL FUNCTIONS — NO LAMBDAS)
// ---------------------------------------------------------
struct Action {
  bool last;
  void (*onPress)();
  bool (*isDown)();
  void update() {
    bool now = isDown();
    if (now && !last) onPress();
    last = now;
  }
};

// ---------------------------------------------------------
// BUTTON-PRESS CHECK FUNCTIONS (RAW FUNCTION POINTER SAFE)
// ---------------------------------------------------------
bool isY() { return Controller1.ButtonY.pressing(); }
bool isX() { return Controller1.ButtonX.pressing(); }
bool isA() { return Controller1.ButtonA.pressing(); }
bool isB() { return Controller1.ButtonB.pressing(); }

// ---------------------------------------------------------
// ACTION CALLBACK FUNCTIONS
// ---------------------------------------------------------

void toggleMag() {
  magOn = !magOn;
}

void cycleArm() {
  currentArm = (currentArm + 1) % 3;
  armAutoMove = true;
  Brain.Screen.printAt(10, 40, "Arm Target: %d   ", armTargets[currentArm]);
}

void cycleNeural() {
  currentNeural = (currentNeural + 1) % 3;
  neuralAutoMove = true;
  Brain.Screen.printAt(10, 60, "Neural Target: %d   ", neuralTargets[currentNeural]);
}

void incrementServo() {
  // Temporary debug action
  Brain.Screen.printAt(10, 80, "Servo Increment Pressed");
}


// ---------------------------------------------------------
// ACTIONS ARRAY
// ---------------------------------------------------------
Action actions[] = {
  {false, toggleMag,   isY},
  {false, cycleNeural, isX},
  {false, cycleArm,    isB},
  {false, incrementServo, isA}
};


// ---------------------------------------------------------
// ARM + NEURAL AUTO CONTROL
// ---------------------------------------------------------

void updateArm() {
  int pos = ArmPot.value(vex::analogUnits::range12bit);  // 0–4095
  int target = armTargets[currentArm];

  if (armAutoMove) {
    int error = target - pos;

    if (abs(error) < 3) {   // within tolerance
      armAutoMove = false;
      ArmMotor.stop();
      return;
    }

    // Simple proportional move
    ArmMotor.spin(vex::directionType::fwd, error * 2, vex::velocityUnits::pct);
  }
}

void updateNeural() {
  int pos = ArmPot.value(vex::analogUnits::range12bit); // example if using same pot
  int target = neuralTargets[currentNeural];

  if (neuralAutoMove) {
    int error = target - pos;

    if (abs(error) < 5) {
      neuralAutoMove = false;
      NeuralMotor.stop();
      return;
    }

    NeuralMotor.spin(vex::directionType::fwd, error * 2, vex::velocityUnits::pct);
  }
}


// ---------------------------------------------------------
// MAIN LOOP
// ---------------------------------------------------------
int main() {

  while (true) {

    // Update button actions
    for (auto &a : actions)
      a.update();

    // Autonomous movements
    updateArm();
    updateNeural();

    vex::task::sleep(20);
  }
}
