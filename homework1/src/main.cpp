//brysons homework
#include "vex.h"

// ---- START VEXCODE CONFIGURED DEVICES ----
// Robot Configuration:
// [Name]               [Type]        [Port(s)]
// Controller1          controller                    
// ---- END VEXCODE CONFIGURED DEVICES ----

using namespace vex;

int main() {
  int X = 0; // X is the right button
  int Y = 0;// Y is the left butyon
  int Answer = X + Y;
while(true){
  if(Controller1.ButtonR1.pressing()){
    X++;
    Brain.Screen.clearScreen();
    Brain.Screen.printAt( 100, 50, "X: %d", X);
  }else if(Controller1.ButtonR1.pressing()){
    X--;
    Brain.Screen.clearScreen();
    Brain.Screen.printAt( 100, 50, "X: %d", X);
  }
  if(Controller1.ButtonL1.pressing()){
    Y++;
    Brain.Screen.clearScreen();
    Brain.Screen.printAt( 100, 70, "Y: %d", Y);
  }else if(Controller1.ButtonL1.pressing()){
    Y--;
    Brain.Screen.clearScreen();
    Brain.Screen.printAt( 100, 70, "Y: %d", Y);
  }
  if(Answer = 69){
    Brain.Screen.clearScreen();
    Brain.Screen.printAt( 100, 30,":} hehe");
  }
  else if(Answer != 69){
    Brain.Screen.clearScreen();
    Brain.Screen.printAt( 100, 30,"Answer: %d", Answer);
  }
  if(X < 0){
    X = X * -X;
  }
  if(Y < 0){
    Y = Y * -Y;
  }
}
}
