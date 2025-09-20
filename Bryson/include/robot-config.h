#include "vex.h"

using namespace vex;

extern brain Brain;

// VEXcode devices
extern controller Controller1;
extern servo Servo1;
extern motor ConvMotor;
extern servo Servo2;

/**
 * Used to initialize code/tasks/devices added using tools in VEXcode Pro.
 * 
 * This should be called at the start of your int main function.
 */
int main() {

int Holding =0;
int Stickpos = Controller1.Axis4.position();

bool Converor = false;
bool Realese = false;

while(true){
 if(Stickpos > -5 && Stickpos < 5){
  Stickpos = 0;   
}
  else if(Stickpos < 95){
    Stickpos = 100;
}
  else if(Stickpos > -95){
    Stickpos = -100;
}
 if(Controller1.ButtonR1.pressing()){
  Holding++;
  Servo1.setPosition(Holding, degrees);
  Brain.Screen.clearScreen();
  Controller1.Screen.clearScreen();
  Brain.Screen.newLine();
  Controller1.Screen.newLine();
  Brain.Screen.print("%d", Holding);
  Controller1.Screen.print("%d", Holding);
}
else if(Controller1.ButtonR2.pressing()){
  Holding--;
  Servo1.setPosition(Holding, degrees);
  Brain.Screen.clearScreen();
  Controller1.Screen.clearScreen();
  Brain.Screen.newLine();
  Controller1.Screen.newLine();
  Brain.Screen.print("%d", Holding);
  Controller1.Screen.print("%d", Holding);
}

if(Controller1.ButtonX.pressing()){
    Converor = !Converor; // toggle the motor state
}
  if(Converor == true){
    ConvMotor.spin(forward, 100,pct);
}
  else{
  ConvMotor.stop();
}
    wait(20, msec);
    if(Controller1.Buttonb.pressing()){
    Realese = !Realese; 
     if(Realese = false){
  Servo2.setPosition(90, pct);
}
 else if(Realese = true){
  Servo2.setPosition(180, pct);
}

 
}
}