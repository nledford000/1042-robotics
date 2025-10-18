#include "vex.h"

using namespace vex;
using signature = vision::signature;
using code = vision::code;

// A global instance of brain used for printing to the V5 Brain screen
brain  Brain;

// VEXcode device constructors
controller Controller1 = controller(primary);
motor ToggleMotor = motor(PORT3, ratio18_1, false);
motor NeuralMotor = motor(PORT4, ratio18_1, false);
pot NeuralPot = pot(Brain.ThreeWirePort.A);
servo ServoOC = servo(Brain.ThreeWirePort.B);
servo ServoPush = servo(Brain.ThreeWirePort.C);
motor LeftMotor = motor(PORT1, ratio18_1, false);
motor RightMotor = motor(PORT2, ratio18_1, false);

// VEXcode generated functions
// define variable for remote controller enable/disable
bool RemoteControlCodeEnabled = true;

/**
 * Used to initialize code/tasks/devices added using tools in VEXcode Pro.
 * 
 * This should be called at the start of your int main function.
 */
void vexcodeInit( void ) {
  // nothing to initialize
}