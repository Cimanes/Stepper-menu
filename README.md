# Stepper-menu
Interactive menu via Serial Monitor to play with stepper motor "28BYJ-48"

 * Cimanes - 30/04/2020
 *  
 * This sketch can be used to move a stepper motor. 
 * It was developed and tested using the Stepper motor "28BYJ-48" and driver module "ULN2003"
 * A power supply (5 VDC) is used to feed the motor.
 * The constant "Rpm_Max" will set the Max speed for the potentiometer. 
 * The rest of speed settings (1 to 8) will be calculated proportionally
 *   
 * There is an interactive menu in the Serial Monitor. 
 * The user can select three modes of operation "A, B, C" (incrasing torque and decreasing consumption)
 * The user can select movement forward or backwards 
 * The user can select the speed (Rpm)
 * The angle of the motor is monitored (Deg) 
 * Each time a command is sent to the Serial monitor, the curent conditions are displayed.
 * 
 * Target mode:
 * The menu allows the user to select a target angle and the motor will move there.
 * The sketch will choose the direction with shortest distance to reach the target angle. 
 * 
 * The interactive menu looks as follows: 
 * 
 * Select direction: 'F' (fwd)    /  'R' (Rear)
 * Select torque   : 'A' (low)    /  'B' (mid) / 'C' (high)
 * Select speed    : Rpm (value)  /  'P' (Potentiometer)
 * --------------------------------------------------------
 * Direction = F / Torque = B / Rpm = 0.00 / Dt = 7000 / Deg = 0.00 / Status = Stop
 * 'S' to start / 'X' to stop / 'M' to refresh menu / 'T' to go to a Target angle
