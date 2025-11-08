#include "vex.h"

using namespace vex;
using signature = vision::signature;
using code = vision::code;

// A global instance of brain used for printing to the V5 Brain screen
brain  Brain;

// VEXcode device constructors
controller Controller1 = controller(primary);
motor LeftMotor = motor(PORT1, ratio18_1, false);
motor RightMotor = motor(PORT2, ratio18_1, false);
motor ArmMotor = motor(PORT3, ratio18_1, false);
motor NeuralMotor = motor(PORT4, ratio18_1, false);
servo ServoOC = servo(Brain.ThreeWirePort.B);
pot ArmPot = pot(Brain.ThreeWirePort.D);
pot NeuralPot = pot(Brain.ThreeWirePort.E);
servo ServoInc = servo(Brain.ThreeWirePort.G);

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