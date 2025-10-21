#include "vex.h"

// ---- START VEXCODE CONFIGURED DEVICES ----
// Robot Configuration:
// [Name]               [Type]        [Port(s)]
// Controller1          controller                    
// ToggleMotor          motor         3               
// NeuralMotor          motor         4               
// NeuralPot            pot           A               
// ServoOC              servo         B               
// ServoPush            servo         C               
// LeftMotor            motor         1               
// RightMotor           motor         2               
// ---- END VEXCODE CONFIGURED DEVICES ----

using namespace vex;

// ================== VARIABLES ==================
int driveDeadband = 15;

int ballSpeed = 100;

int magOpen = 100;
int magClosed = 0;

int panelForward = 80;
int panelBack = 0;

double neuralTargetDeg = 1800.0;
int neuralMotorSpeed = 60;

bool ballToggle = false;
bool magToggle = false;
bool panelToggle = false;
bool neuralBusy = false;

void toggleBallAction() {
  ballToggle = !ballToggle;
  if (ballToggle)
    ToggleMotor.spin(forward, ballSpeed, pct);
  else
    ToggleMotor.stop();
}

int neuralForwardToTarget() {
  if (neuralBusy) return 0;
  neuralBusy = true;

  NeuralMotor.spin(forward, neuralMotorSpeed, pct);
  while (NeuralPot.angle(degrees) < neuralTargetDeg)
    wait(10, msec);
  NeuralMotor.stop(hold);

  neuralBusy = false;
  return 0;
}

void startNeuralAction() {
  if (!neuralBusy) thread t(neuralForwardToTarget);
}

void toggleMagAction() {
  magToggle = !magToggle;
  ServoOC.setPosition((magToggle) ? magOpen : magClosed, degrees);
}

void togglePanelAction() {
  panelToggle = !panelToggle;
  ServoPush.setPosition((panelToggle) ? panelForward : panelBack, degrees);
}

//EVENT STRUCT
struct Action {
  bool lastState;
  void (*onPress)();
  const controller::button *button;
  void update() {
    bool now = button->pressing();
    if (now && !lastState && onPress) onPress();
    lastState = now;
  }
};

//MAIN
int main() {
  vexcodeInit();

  Action actions[4];

  actions[0] = {false, toggleBallAction, &Controller1.ButtonX};
  actions[1] = {false, startNeuralAction, &Controller1.ButtonB};
  actions[2] = {false, toggleMagAction, &Controller1.ButtonY};
  actions[3] = {false, togglePanelAction, &Controller1.ButtonA};

  while (true) {
    int leftSpeed = Controller1.Axis3.position();
    int rightSpeed = Controller1.Axis2.position();

    LeftMotor.spin(forward, (abs(leftSpeed) >= driveDeadband) ? leftSpeed : 0, pct);
    RightMotor.spin(forward, (abs(rightSpeed) >= driveDeadband) ? rightSpeed : 0, pct);

    for (int i = 0; i < 4; ++i)
      actions[i].update();
  }
}
