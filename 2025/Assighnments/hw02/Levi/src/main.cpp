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
int z = 0;
int xyz;
int badges = 0;

bool xsel = true;
bool ysel = false;
bool zsel = false;
bool can42 = true;
bool flag = false;
bool pflag = true;
bool b10 = false;
bool b25 = false;
bool b50 = false;
bool b100 = false;
bool b250 = false;
bool b500 = false;
bool b1000 = false;

void update() {
  Brain.Screen.clearScreen();
  Brain.Screen.setCursor(1, 5);
  Brain.Screen.print("X = %d", x);
  Brain.Screen.setCursor(2, 5);
  Brain.Screen.print("Y = %d", y);
  Brain.Screen.setCursor(3, 5);
  Brain.Screen.print("Z =%d", z);
  xyz = x + y + z;
  Brain.Screen.setCursor(4, 5);
  Brain.Screen.print("Answer: %d", xyz);
  Brain.Screen.setCursor(5, 5);
  Brain.Screen.print("Badges: %d/7", badges);
  pflag = false;
}

void incX() {
  x++;
  wait(200, msec);
  flag = false;
}
void incY() {
  y++;
  wait(200, msec);
  flag = false;
}
void incZ() {
  z++;
  wait(200, msec);
  flag = false;
}
void drsX() {
  if(x >= 0) {
    x--;
  }else{
    x = 0;
  }
  wait(200, msec);
  flag = false;
}
void drsY() {
  if(y >= 0){
    y--;
  }else{
    y = 0;
  }
  wait(200, msec);
  flag = false;
}
void drsZ() {
  if(z >= 0) {
    z--;
  }else{
    z = 0;
  }
  wait(200, msec);
  flag = false;
}
void selR() {
  if(xsel) {
    xsel = false;
    ysel = true;
    zsel = false;
  }else if(ysel) {
    xsel = false;
    ysel = false;
    zsel = true;
  }
  wait(200, msec);
}
void selL() {
  if(ysel) {
    xsel = true;
    ysel = false;
    zsel = false;
  }else if(zsel) {
    xsel = false;
    ysel = true;
    zsel = false;
  }
  wait(200, msec);
}
void reset() {
  x = 0;
  y = 0;
  z = 0;
  wait(200, msec);
  pflag = true;
}


int main() {
  while(true) {
    

    if(xsel && !flag && Controller1.ButtonUp.pressing()) {
      flag = true;
      incX();
      pflag = true;
    }
    if(xsel && !flag && Controller1.ButtonDown.pressing()) {
      flag = true;
      drsX();
      pflag = true;
    }
    if(ysel && !flag && Controller1.ButtonUp.pressing()) {
      flag = true;
      incY();
      pflag = true;
    }
    if(ysel && !flag && Controller1.ButtonDown.pressing()) {
      flag = true;
      drsY();
      pflag = true;
    }
    if(zsel && !flag && Controller1.ButtonUp.pressing()) {
      flag = true;
      incZ();
      pflag = true;
    }
    if(zsel && !flag && Controller1.ButtonDown.pressing()) {
      flag = true;
      drsZ();
      pflag = true;

    }
    if(!b10 && !flag && xyz == 10 && badges != 7){
      b10 = true;
      flag = true;
      badges++;
      Brain.Screen.clearScreen();
      Brain.Screen.setCursor(6, 5);
      Brain.Screen.print("10 reached! +1 badge.");
      wait(1, sec);
      pflag = true;
    }
    if(!b25 && !flag && xyz == 25 && badges != 7){
      b25 = true;
      flag = true;
      badges++;
      Brain.Screen.clearScreen();
      Brain.Screen.setCursor(6, 5);
      Brain.Screen.print("25 reached! +1 badge.");
      wait(1, sec);
      pflag = true;
    }
    if(!b50 && !flag && xyz == 50 && badges != 7){
      b50 = true;
      flag = true;
      badges++;
      Brain.Screen.clearScreen();
      Brain.Screen.setCursor(6, 5);
      Brain.Screen.print("50 reached! +1 badge.");
      wait(1, sec);
      pflag = true;
    }
    if(!b100 && !flag && xyz == 100 && badges != 7){
      b100 = true;
      flag = true;
      badges++;
      Brain.Screen.clearScreen();
      Brain.Screen.setCursor(6, 5);
      Brain.Screen.print("100 reached! +1 badge.");
      wait(1, sec);
      pflag = true;
    }
    if(!b250 && !flag && xyz == 250 && badges != 7){
      b250 = true;
      flag = true;
      badges++;
      Brain.Screen.clearScreen();
      Brain.Screen.setCursor(6, 5);
      Brain.Screen.print("250 reached! +1 badge.");
      wait(1, sec);
      pflag = true;
    }
    if(!b500 && !flag && xyz == 500 && badges != 7){
      b500 = true;
      flag = true;
      badges++;
      Brain.Screen.clearScreen();
      Brain.Screen.setCursor(6, 5);
      Brain.Screen.print("500 reached! +1 badge.");
      wait(1, sec);
      pflag = true;
    }
    if(!b1000 && !flag && xyz == 1000 && badges != 7){
      b1000 = true;
      flag = true;
      badges++;
      Brain.Screen.clearScreen();
      Brain.Screen.setCursor(6, 5);
      Brain.Screen.print("1000 reached! +1 badge.");
      wait(1, sec);
      pflag = true;
    }

    Controller1.ButtonRight.pressed(selR);
    Controller1.ButtonLeft.pressed(selL);
    Controller1.ButtonA.pressed(reset);
    
    if(pflag){
      update();
    }

    if(x + y == 42 && can42) {
      can42 = false;
      Brain.Screen.clearScreen();
      Brain.Screen.setCursor(6, 5);
      Brain.Screen.print("The answer to everything is 42");
      wait(1, sec);
      pflag = true;
    }else{
      can42 = true;
    }

  }
}