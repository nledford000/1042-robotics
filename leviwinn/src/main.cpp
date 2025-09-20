/*----------------------------------------------------------------------------*/
/*                                                                            */
/*    Module:       main.cpp                                                  */
/*    Author:       C:\Users\leviw                                            */
/*    Created:      Sat Sep 20 2025                                           */
/*    Description:  V5 project                                                */
/*                                                                            */
/*----------------------------------------------------------------------------*/

// ---- START VEXCODE CONFIGURED DEVICES ----
// Robot Configuration:
// [Name]               [Type]        [Port(s)]
// Controller1          controller                    
// BallRlease           servo         A               
// BoxTurn              servo         B               
// ConvMotor            motor         1               
// LeftWhls             motor_group   2, 3            
// RightWhls            motor_group   4, 5            
// ---- END VEXCODE CONFIGURED DEVICES ----

#include "vex.h"

using namespace vex;

int i = 0;
int joyL = Controller1.Axis3.position(pct);
int joyR = Controller1.Axis2.position(pct);

bool convoff = true;
bool is180 = true;

void Conveyor() {

  if(convoff){
    ConvMotor.spin(forward, 70, pct);
    convoff = false;
  }else{
    ConvMotor.spin(forward, 0, pct);
    convoff = true;
  }

}

void release() {

  if(is180){
    BallRlease.setPosition(90, degrees);
    is180 = false;
  }else{
    BallRlease.setPosition(180, degrees);
    is180 = true;
  }

}

int main() {

  BoxTurn.setPosition(0, degrees);
  while(true) {
    
    Controller1.ButtonX.pressed(Conveyor);
    Controller1.ButtonB.pressed(release);

    if(Controller1.ButtonR1.pressing()) {
      BoxTurn.setPosition(i, degrees);
      i++;
    }
    if(Controller1.ButtonR2.pressing()) {
      BoxTurn.setPosition(i, degrees);
      i--;
    }
    if(joyL >= 95) {
      LeftWhls.spin(forward, 100, pct);
    }else if(joyL <= -95) {
      LeftWhls.spin(forward, -100, pct);
    }else if(joyL <= 5 || joyL >= -5) {
      LeftWhls.spin(forward, 0, pct);
    }else{
      LeftWhls.spin(forward, joyL, pct);
    }
    if(joyR >= 95) {
      RightWhls.spin(forward, 100, pct);
    }else if(joyR <= -95) {
      RightWhls.spin(forward, -100, pct);
    }else if(joyR<= 5 || joyR >= -5) {
      RightWhls.spin(forward, 0, pct);
    }else{
      RightWhls.spin(forward, joyR, pct);
    }

  }
  
}
