/* Cimanes - 30/04/2020
 *  
 * This sketch can be used to move a stepper motor. 
 * It was developed and tested using the Stepper motor "28BYJ-48" and driver module "ULN2003"
 * A power supply (5 VDC) is used to feed the motor.
 * The rest of speed settings (1 to 8) will be calculated proportionally
 * The constant "Rpm_Max" will set the Max speed for the potentiometer. 
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
 * Select direction: 'F' (fwd)  /  'R' (Rear)
 * Select torque   : 'A' (low)  /  'B' (mid) / 'C' (high)
 * Select speed    : '0' (min) ... '9' (max) / 'T' (Turbo) / 'P' (Pot control)";
 * --------------------------------------------------------
 * Direction = F / Torque = B / Rpm = 0.00 / Dt = 7000 / Deg = 0.00 / Status = Stop
 * 'S' to start / 'X' to stop / 'M' to refresh menu / 'T' to go to a Target angle
 */
 
// define step sequences in matrix format:

const bool seqA [4][4] =            // A. single phase sequence (lower torque, lower consumption)
{
  {HIGH, LOW,  LOW,  LOW },
  {LOW,  HIGH, LOW,  LOW },
  {LOW,  LOW,  HIGH, LOW },
  {LOW,  LOW,  LOW,  HIGH},
};

const bool seqB [8][4] =            // B. "single/double" sequence (mid torque, smoother operation, mid consumption)
{
  {HIGH, LOW,  LOW,  LOW },
  {HIGH, HIGH, LOW,  LOW },
  {LOW,  HIGH, LOW,  LOW },
  {LOW,  HIGH, HIGH, LOW },
  {LOW,  LOW,  HIGH, LOW },
  {LOW,  LOW,  HIGH, HIGH},
  {LOW,  LOW,  LOW,  HIGH},
  {HIGH, LOW,  LOW,  HIGH}
};

const bool seqC [4][4] =            // C. double phase sequence (max torque, higher consumption)
{
  {HIGH, HIGH, LOW,  LOW },
  {LOW,  HIGH, HIGH, LOW },
  {LOW,  LOW,  HIGH, HIGH},
  {HIGH, LOW,  LOW,  HIGH}
};

// Define Connections:
//            ULN2003:  IN1, IN2, IN3, IN4
const byte IN[4] =     { 8,   9,  10,  11 } ;     // Define pins connected to the coils
const byte potAI = 0                        ;     // Define pin for analog input from potentiometer (speed control)

// Define motor specs:
const int Steps   = 4096    ;     // Number of steps for "8 step sequence". 
const float Rpm_Max  =  20  ;     // Max Rpm (recommended 20 for 28BYJ-48 stepper motor)

// Define initial conditions
bool Pot          = LOW   ;       // This variable will be HIGH when using potentiometer to control the speed.
String Input      = "X"   ;       // Character array received from Serial Monitor
char Initial      = 'X'   ;       // Initial character of the String "Input"
char Direction    = 'F'   ;       // Turn direction (Forward / Rear); Forward by default
char Torque       = 'B'   ;       // Torque selection (A / B / C). Mid torque (B) by default
byte Sequence     =  8    ;       // Steps in the sequence ($ / 8). 8 steps by default
int Target        =  0    ;       // Target angle (degrees)
String Status     = "Stop";       // String to define if the engine is running or stopped
float Rpm         =  0    ;       // Current angular speed (Rpm)
float Deg         =  0    ;       // Current angle(degrees)
float Diff        =  0    ;       // Difference (degrees) between target and current angle
float Deadband    =  0.5  ;       // Deadband (degrees) to chase target (note: each step is 0,7 deg approx)
float KDt         = 14648 ;       // Constant based on Steps. Used to calculate Dt (KDt = 60.000 * 1000 / Steps)
unsigned int Dt   = 7000  ;       // Delay time (microseconds) between steps. It is inversely proportional to speed

void setup()
{
  for (int i = 0; i < 4; i++) pinMode(IN[i], OUTPUT);     // Set all pins as Outputs:
  KDt = 60000 / Steps * 1000                        ;     // Calculate KDt

// Display initial menu (user selectable options) and initial conditions:
  Serial.begin(9600);
  Serial.println("--------------------------------------------------------"); 
  Serial.println("Select direction: 'F' (fwd)  /  'R' (Rear)");
  Serial.println("Select torque   : 'A' (low)  /  'B' (mid) / 'C' (high)");
  Serial.println("Select speed    : Rpm value  /  'P' (Potentiometer)");
  Serial.println("--------------------------------------------------------"); 
  Serial.println("Direction = " + String(Direction) + " / Torque = " + String(Torque) + 
                 " / Rpm = " + String(Rpm) + " / Dt = " + String(Dt) + 
                 " / Deg = " + String(Deg) + " / Status = " + String(Status));
  Serial.println("'S' to start / 'X' to stop / 'M' to refresh menu / 'T' to go to a target angle");
  Serial.println();
}
 
void loop() { 
  if(Serial.available()) 
  {
    Input = Serial.readStringUntil('\n');               // Read incoming string from Serial Monitor
    if (Input == "T") goTarget();                         // Selection "go to target angle"
    else menu();                                        // Selection of menu to define running conditions
    Dt = KDt / Rpm;                                     // Calculate time delay

// Print current conditions:
    Serial.println("Direction = " + String(Direction) + " / Torque = " + String(Torque) + 
                   " / Rpm = " + String(Rpm) + " / Dt = " + String(Dt) + 
                   " / Deg = " + String(Deg) + " / Status = " + String(Status));
// Print options:
    Serial.println("'S' to start / 'X' to stop / 'M' to refresh menu / 'T' to go to a target angle");
    Serial.println();
  }

  if (Status == "Stop" or Rpm == 0) return;             // Stop if required or if selected Rpm is "0"
  if (Torque == 'A') { Sequence = 4;  Dt *= 2; A(); }   // Low torque selection (4 steps, Double delay)
  if (Torque == 'B') { Sequence = 8;           B(); }   // Mid torque selection (8 steps)
  if (Torque == 'C') { Sequence = 4;  Dt *= 2; C(); }   // Low torque selection (4 steps, Double delay)
}

void menu() {
// Select running options: 
  Initial = Input.charAt(0);
  if (Input == "M") {
    Serial.println("--------------------------------------------------------"); 
    Serial.println("Select direction: 'F' (fwd)    /  'R' (Rear)");
    Serial.println("Select torque   : 'A' (low)    /  'B' (mid) / 'C' (high)");
    Serial.println("Select speed    : Rpm (value)  /  'P' (Potentiometer)");
    Serial.println("--------------------------------------------------------"); 
  }
  if (Input == "X")                                 Status    = "Stop";
  if (Input == "S")                                 Status    = "Run" ;
  if (Input == "A" or Input == "B" or Input == "C") Torque    = Input.charAt(0);    // Torque selection
  if (Input == "F" or Input == "R")                 Direction = Input.charAt(0);    // Direction selection
  if (Input == "P") {                                                               // Pot control for speed
    Pot = HIGH;  
    Rpm = analogRead(potAI) * Rpm_Max / 1093; 
  }
  if (isDigit(Initial)) { 
    Pot = LOW;
    Rpm = Input.toFloat();
    if (Rpm > Rpm_Max) Serial.println("Speed too high");
  }
}

void goTarget() {
start:
// The following four lines (147 to 10) are optional, to use moderate speed and mid torque chasing target.
// They can be bypassed to use the pre-existing speed and torque selection:
// ----------------
  Sequence = 8;                                     // Select mid Torque
  Rpm      = Rpm_Max / 2;                           // Normal speed (50% Rpm)
  Dt       = KDt / Rpm          ;                   // Calculate time delay for 10 Rpm.
// ----------------
  Serial.println("Target mode; Deg = " + String(Deg));    
  Serial.println("Enter 'X' to exit Target mode");
  Serial.print("Enter target (0 to 360 deg): ");    // Prompt for angle target
  while (!Serial.available());                      // Wait to receive the serial input
  Input  = Serial.readStringUntil('\n');            // Read the angle target
  if (Input == "X") {                               // Exit Target mode when user enters "X"
    Serial.println("Exit"); 
    return;
  }
  Target = Input.toInt();                           // Convert the received "string" to integer
  Diff   = abs(Target - Deg);                       // Calculate difference
  Serial.println(Target);                           // Print the selected target 
  Serial.println("--------------------------");
  
  if (Diff < Deadband) return;                      // Exit if the angle is already close enough to the Target
  if ( (Diff < 180 and Deg < Target) or (Diff > 180 and Deg > Target) ) {
    do {
      for (byte i = 0; i < Sequence; i++) {
        for (byte j = 0; j < 4; j++)  digitalWrite(IN[j], seqB[i][j]); 
        delayMicroseconds(Dt);
      }
      Deg = Deg + 2880 / float(Steps);                    // Increase angle (8 * 360 / Steps = 2880 / Steps)
      if (Deg > 360)     Deg = Deg - 360;
      if (Diff < 1 and Diff < abs(Target - Deg)) return;  // Stop if difference increases (if deadband was set too tight)
      Diff = abs(Target - Deg);                           // Calculate new difference
    }
    while (Diff > Deadband and !Serial.available());      // Stop when difference is lower than deadband or user enters a command
    if (Serial.available()) Serial.println("Halted by user");
    goto start;
  }  
    
  if ( (Diff < 180 and Deg > Target) or (Diff > 180 and Deg < Target)) {
    do {
      for (byte i = 0; i < Sequence; i++) {
        for (byte j = 0; j < 4 ; j++)  digitalWrite(IN[j], seqB[i][3-j]); 
        delayMicroseconds(Dt);
      }
      Deg = Deg - 2880 / float(Steps);
      if (Deg < 0)     Deg = Deg + 360;
      if (Diff < 1 and Diff < abs(Target - Deg)) return;  // Stop if difference increases (if deadband was set too tight)
      Diff = abs(Target - Deg);                           // Calculate new difference
    }
    while (Diff > Deadband and !Serial.available());      // Stop when difference is lower than deadband or user enters a command
    if (Serial.available()) Serial.println("Halted by user");
    goto start;
  }
}

void A() {

  if (Direction == 'F')
    do {
      if (Pot == HIGH)  Dt = KDt / Rpm_Max * 1023 / analogRead(potAI);
      for (byte i = 0; i < Sequence; i++) {
        for (byte j = 0; j < 4; j++)  digitalWrite(IN[j], seqA[i][j]);
        delayMicroseconds(Dt);
      }
      Deg = Deg + 2880 / float(Steps);                    // Increase angle (8 * 360 / Steps = 2880 / Steps)
      if (Deg >= 360) Deg = Deg - 360;
    }
    while (!Serial.available());

  if (Direction == 'R')
    do {
      if (Pot == HIGH)  Dt = KDt / Rpm_Max * 1023 / analogRead(potAI) ;
      for (byte i = 0; i < Sequence; i++) {
        for (byte j = 0; j < 4 ; j++)  digitalWrite(IN[j], seqA[i][3-j]); 
        delayMicroseconds(Dt);
      }
      Deg = Deg - 2880 / float(Steps);                    // Decrease angle (8 * 360 / Steps = 2880 / Steps)
      if (Deg < 0)     Deg = Deg + 360;
    }
    while (!Serial.available());
}

void B() {
  if (Direction == 'F')
    do {
      if (Pot == HIGH)  Dt = KDt / Rpm_Max * 1023 / analogRead(potAI) / 2  ;   // half Dt to compensate for calculation time
      for (byte i = 0; i < Sequence; i++) {
        for (byte j = 0; j < 4; j++)  digitalWrite(IN[j], seqB[i][j]); 
        delayMicroseconds(Dt);
       }
      Deg = Deg + 2880 / float(Steps);                    // Increase angle (8 * 360 / Steps = 2880 / Steps)
      if (Deg > 360)     Deg = Deg - 360;
    }
    while (!Serial.available()); 

  if (Direction == 'R')
    do {
      if (Pot == HIGH)  Dt = KDt / Rpm_Max * 1023 / analogRead(potAI) / 2  ;   // half Dt to compensate for calculation time
      for (byte i = 0; i < Sequence; i++) {
        for (byte j = 0; j < 4 ; j++)  digitalWrite(IN[j], seqB[i][3-j]); 
        delayMicroseconds(Dt);
      }
      Deg = Deg - 2880 / float(Steps);                    // Decrease angle (8 * 360 / Steps = 2880 / Steps)
      if (Deg < 0)     Deg = Deg + 360;
      }
    while (!Serial.available()); 
}

void C() {
  if (Direction == 'F')
    do {
      if (Pot == HIGH)  Dt = KDt / Rpm_Max * 1023 / analogRead(potAI) ;   
      for (byte i = 0; i < Sequence; i++) {
        for (byte j = 0; j < 4; j++)  digitalWrite(IN[j], seqC[i][j]); 
        delayMicroseconds(Dt);
      }
      Deg = Deg + 2880 / float(Steps);                    // Increase angle (8 * 360 / Steps = 2880 / Steps)
      if (Deg > 360)     Deg = Deg - 360;
    }
    while (!Serial.available()); 

  if (Direction == 'R')
    do {
      if (Pot == HIGH)  Dt = KDt / Rpm_Max * 1023 / analogRead(potAI) ;   // half Dt to compensate for calculation time
      for (byte i = 0; i < Sequence; i++) {
        for (byte j = 0; j < 4 ; j++)  digitalWrite(IN[j], seqC[i][3-j]); 
        delayMicroseconds(Dt);
      }
      Deg = Deg - 2880 / float(Steps);                    // Decrease angle (8 * 360 / Steps = 2880 / Steps)
      if (Deg < 0)     Deg = Deg + 360;
    }
    while (!Serial.available()); 
}
