using namespace vex;

extern brain Brain;

// VEXcode devices
extern controller Controller1;
extern servo BallRlease;
extern servo BoxTurn;
extern motor ConvMotor;
extern motor_group LeftWhls;
extern motor_group RightWhls;

/**
 * Used to initialize code/tasks/devices added using tools in VEXcode Pro.
 * 
 * This should be called at the start of your int main function.
 */
void  vexcodeInit( void );