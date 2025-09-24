/*----------------------------------------------------------------------------*/
/*                                                                            */
/*    Module:       main.cpp                                                  */
/*    Author:       C:\Users\leviw                                            */
/*    Created:      Tue Sep 23 2025                                           */
/*    Description:  V5 project                                                */
/*                                                                            */
/*----------------------------------------------------------------------------*/

// ---- START VEXCODE CONFIGURED DEVICES ----
// Robot Configuration:
// [Name]               [Type]        [Port(s)]
// Controller1          controller                    
// ---- END VEXCODE CONFIGURED DEVICES ----

#include "vex.h"

using namespace vex;

int x = 0;
int y = 0;
int xy;

bool xsel = true;
bool ysel = false;
bool can42 = true;
bool flag = true;

void update() {
  Brain.Screen.clearScreen();
  Brain.Screen.setCursor(5,5);
  Brain.Screen.print("X = %d", x);
  Brain.Screen.setCursor(5, 10);
  Brain.Screen.print("Y = %d", y);
  xy = x + y;
  Brain.Screen.setCursor(6,5);
  Brain.Screen.print("Answer: %d", xy);
  flag = false;
}

void incX() {
  x++;
  flag = true;
}
void incY() {
  y++;
  flag = true;
}
void drsX() {
  if(x >= 0) {
    x--;
  }else{
    x = 0;
  }
  flag = true;
}
void drsY() {
  if(y >= 0){
    y--;
  }else{
    y = 0;
  }
  flag = true;
}
void selR() {
  if(xsel) {
    xsel = false;
    ysel = true;
    Controller1.ButtonUp.pressed(incY);
    Controller1.ButtonDown.pressed(drsY);
  }
}
void selL() {
  if(ysel) {
    xsel = true;
    ysel = false;
    Controller1.ButtonUp.pressed(incX);
    Controller1.ButtonDown.pressed(drsX);
  }
}
void reset() {
  x = 0;
  y = 0;
}


int main() {

  Controller1.ButtonRight.pressed(selR);
  Controller1.ButtonLeft.pressed(selL);
  Controller1.ButtonA.pressed(reset);
  Controller1.ButtonUp.pressed(incX);
  Controller1.ButtonDown.pressed(drsX);
  
  while(true) {
    if(flag) {
      update();
    }
    if(x + y == 42 && can42) {
      Brain.Screen.clearScreen();
      Brain.Screen.print("The answer to everything is 42");
      can42 = false;
    }else{
      can42 = true;
    }

  }
}