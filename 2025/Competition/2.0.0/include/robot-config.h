using namespace vex;

extern brain Brain;

// VEXcode devices
extern controller Controller1;
extern motor ToggleMotor;
extern motor NeuralMotor;
extern pot NeuralPot;
extern servo ServoOC;
extern servo ServoPush;
extern motor LeftMotor;
extern motor RightMotor;

/**
 * Used to initialize code/tasks/devices added using tools in VEXcode Pro.
 * 
 * This should be called at the start of your int main function.
 */
void  vexcodeInit( void );