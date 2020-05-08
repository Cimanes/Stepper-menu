# Stepper-menu
Interactive menu via Serial Monitor to play with stepper motor "28BYJ-48"

/* Cimanes - 30/04/2020
 *  
 * This sketch can be used to move a stepper motor. 
 * It was developed and tested using the Stepper motor "28BYJ-48" and driver module "ULN2003"
 * A power supply (5 VDC) is used to feed the motor.
 * The constant "Rpm_9" will set the "max speed". 
 * The rest of speed settings (1 to 8) will be calculated proportionally
 * The constant "Rpm_T" will set the "turbo speed". 
 * "Rpm_T" does not necessarily need to follow the proportion of the rest of speed settings.
 *   
 * There is an interactive menu in the Serial Monitor. 
 * The user can select three modes of operation "A, B, C" (incrasing torque and decreasing consumption)
 * The user can select movement forward or backwards 
 * The user can select the speed (levels 0 to 9, and Turbo)
 * The angle of the motor is monitored (Deg) 
 * Each time a command is sent to the Serial monitor, the curent conditions are displayed.
 * The menu allows the user to select a target angle and the motor will move there.
 * The sketch will choose the direction with shortest distance to reach the target angle. 
 * 
 * The interactive menu looks as follows: 
 * 
 * Select direction: 'F' (fwd)  /  'R' (Rear)
 * Select torque   : 'A' (low)  /  'B' (mid) / 'C' (high)
 * Select speed    : '0' (min) ... '9' (max) / 'T' (Turbo)
 * --------------------------------------------------------
 * Direction = F / Torque = B / Speed = 0 / Rpm = 0.00 / Dt = 7000 / Deg = 0.00 / Status = Stop
 * 'S' to start / 'X' to stop / 'M' to refresh menu / 'G' to go to a target angle
 */
