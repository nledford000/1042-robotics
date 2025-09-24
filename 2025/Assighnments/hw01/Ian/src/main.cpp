#include "vex.h"
using namespace vex;
int main(){
  int toggle;
  bool = toggle;
  bool motorOn = false;
  bool buttonPreviouslyPressed = false;
  while(true){
  int Controller1;
  if(Controller1 < 5 && Controller1 > -5){
   Motor1.spin(forward, 0, percent);
  if(Controller1 < 95)
   Motor1.spin(forward, 100, percent);
  if(Controller1 > -95)
   Motor1.spin(reverse, -100, percent);{
  if(Controller1.ButtonX.pressing()){
    ConveyorM.spin(forward, 50, percent);{
      if(ButtonR1.pressing.5sec)
      Motor3.spin(forward, 10, percent);
      if(ButtonR1.pressing.10sec)
      Motor3.spin(forward, 20, percent);
      if(ButtonR1.pressing.20sec)
      Motor3.spin(forward, 40, percent);
     
      if(ButtonR2.pressing.5sec)
      Motor3.spin(reverse, 10, percent);
      if(ButtonR2.pressing.10sec)
      Motor3.spin(reverse, 20, percent);
      if(ButtonR2.pressing.20sec)
      Motor3.spin(reverse, 40, percent);{

        if(ButtonB.pressing)
        toggle.Motor4(forward, 90, percent);
        toggle.Motor4(forward, 180, percent);
        wait(30, msec);

      }
     }
    }
    
    }
   }
  }
}