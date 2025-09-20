#include "vex.h"

using namespace vex;
using signature = vision::signature;
using code = vision::code;

// A global instance of brain used for printing to the V5 Brain screen
brain  Brain;

// VEXcode device constructors
controller Controller1 = controller(primary);
servo BallRlease = servo(Brain.ThreeWirePort.A);
servo BoxTurn = servo(Brain.ThreeWirePort.B);
motor ConvMotor = motor(PORT1, ratio36_1, false);
motor LeftWhlsMotorA = motor(PORT2, ratio18_1, false);
motor LeftWhlsMotorB = motor(PORT3, ratio18_1, false);
motor_group LeftWhls = motor_group(LeftWhlsMotorA, LeftWhlsMotorB);
motor RightWhlsMotorA = motor(PORT4, ratio18_1, false);
motor RightWhlsMotorB = motor(PORT5, ratio18_1, false);
motor_group RightWhls = motor_group(RightWhlsMotorA, RightWhlsMotorB);

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