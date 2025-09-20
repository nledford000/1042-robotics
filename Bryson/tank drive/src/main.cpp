/*----------------------------------------------------------------------------*/
/*                                                                            */
/*    Module:       main.cpp                                                  */
/*    Author:       C:\Users\bryso                                            */
/*    Created:      Tue Sep 16 2025                                           */
/*    Description:  V5 project                                                */
/*                                                                            */
/*----------------------------------------------------------------------------*/

// ---- START VEXCODE CONFIGURED DEVICES ----
// Robot Configuration:
// [Name]               [Type]        [Port(s)]
// Controller1          controller                    
// Servo1               servo         A               
// ConvMotor            motor         1               
// ---- END VEXCODE CONFIGURED DEVICES ----

#include "vex.h"

using namespace vex;
int main() {

int Holding =0;
int Stickpos = Controller1.Axis4.position();

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
 
}
}