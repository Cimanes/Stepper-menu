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

// Define pins connected to the coils
// Connection ULN2003:  IN1, IN2, IN3, IN4
const byte IN[4] =     { 8,   9,  10,  11 } ;

// Define motor specs:
const int Steps   = 4096   ;        // Number of steps for "8 step sequence". 
const float Rpm_9 = 18     ;        // Rpm for max speed selection "9" (Max recommended "18")
const float Rpm_T = 20     ;        // Rpm for "Turbo" selection (Max recommended "20")

// Define initial conditions
char Input        = 'X'    ;        // Character received from Serial Monitor for menu
String str        = "0"    ;        // String received from Serial Monitor for angle target
char Direction    = 'F'    ;        // Forward direction by default
char Torque       = 'B'    ;        // Mid torque by default
byte Sequence     =  8     ;        // 8 steps by default
int Speed         =  0     ;        // Speed level: "0" by default
unsigned int Dt   = 7000   ;        // Delay time (microseconds) between steps. It is inversely proportional to speed
String Status     = "Stop" ;        // String to define if the engine is running or stopped
float Rpm         =  0     ;        // Current angular speed (Rpm)
float Deg         =  0     ;        // Current angle (degrees)
int Target        =  0     ;        // Target angle (degrees)
float Diff        =  0     ;        // Difference (degrees) between target and current angle
float Deadband    =  0.4   ;        // Deadband (degrees) to chase target (note: each step is 0,7 deg approx)
String inTarget   = "0"    ;        // String input from Serial Monitor for Target Angle

void setup()
{
// Set all pins as Outputs:
  for (int i = 0; i < 4; i++) pinMode(IN[i], OUTPUT);

// Display initial menu (user selectable options) and initial conditions:
  Serial.begin(9600);
  Serial.println("--------------------------------------------------------"); 
  Serial.println("Select direction: 'F' (fwd)  /  'R' (Rear)");
  Serial.println("Select torque   : 'A' (low)  /  'B' (mid) / 'C' (high)");
  Serial.println("Select speed    : '0' (min) ... '9' (max) / 'T' (Turbo)");
  
  Serial.println("--------------------------------------------------------"); 
  Serial.println("Direction = " + String(Direction) + " / Torque = " + String(Torque) + 
                 " / Speed = " + String(Speed) + " / Rpm = " + String(Rpm) + " / Dt = " + String(Dt) + 
                 " / Deg = " + String(Deg) + " / Status = " + String(Status));

  Serial.println("'S' to start / 'X' to stop / 'M' to refresh menu / 'G' to go to a target angle");
  Serial.println();
}
 
void loop() { 
  if(Serial.available()) 
  {
    Input = Serial.read();
    if (Input == 'G') goTarget();                           // Selection "go to target angle"
    else menu();                                            // Selection of menu to define running conditions
    Dt = 60000L * 1000 / Rpm / Steps;                       // Calculate time delay
        
    Serial.println("Direction = " + String(Direction) + " / Torque = " + String(Torque) + 
                   " / Speed = " + String(Speed) + " / Rpm = " + String(Rpm) + " / Dt = " + String(Dt) + 
                   " / Deg = " + String(Deg) + " / Status = " + String(Status));
    Serial.println("'S' to start / 'X' to stop / 'M' to refresh menu / 'G' to go to a target angle");
    Serial.println();
  }
  if (Status == "Stop" or Speed == 0) return;           // Stop if required or if selected speed is "0"
  if (Torque == 'A') { Sequence = 4;  Dt *= 2; A(); }   // Low torque selection (4 steps, Double delay)
  if (Torque == 'B') { Sequence = 8;           B(); }   // Mid torque selection (8 steps)
  if (Torque == 'C') { Sequence = 4;  Dt *= 2; C(); }   // Low torque selection (4 steps, Double delay)
}

void menu() {
// Select running options:  
  if (Input == 'M') {
    Serial.println("--------------------------------------------------------"); 
    Serial.println("Select direction: 'F' (fwd)  /  'R' (Rear)");
    Serial.println("Select torque   : 'A' (low)  /  'B' (mid) / 'C' (high)");
    Serial.println("Select speed    : '0' (min) ... '9' (max) / 'T' (Turbo)");
    Serial.println("--------------------------------------------------------"); 
  }
  if (Input == 'X')                                 Status    = "Stop";
  if (Input == 'S')                                 Status    = "Run" ;
  if (Input == 'A' or Input == 'B' or Input == 'C') Torque    = Input;                    // Torque selection
  if (Input == 'F' or Input == 'R')                 Direction = Input;                    // Direction selection
  if (Input == 'T') { Speed = 10; Rpm = Rpm_T; }                                          // Turbo speed (Rpm_T)
  if (Input >= '0' and Input <= '9') { Speed = Input - '0'; Rpm = Speed * (Rpm_9 / 9); }  // Normal speed (0 to Rpm_9)
}
  
void goTarget() {
// The following four lines (147 to 10) are optional, to use moderate speed and mid torque chasing target.
// They can be bypassed to use the pre-existing speed and torque selection:
// ----------------
  Sequence = 8;                                     // Select mid Torque
  Speed    = 4;                                     // Select mid Speed (5)
  Rpm      = Speed * (Rpm_9 / 9);                   // Normal speed (0 to Rpm_9)
  Dt       = 60000L * 1000 / Rpm / Steps;           // Calculate time delay for Speed = 5.
// ----------------
    
  Serial.println("Enter target (0 to 360 deg)");    // Prompt for angle target
  while (!Serial.available());                      // Wait to receive the serial input
  String str = Serial.readStringUntil('\n');        // Read the angle target
  Target = str.toInt();                             // Convert the received "string" to integer
  Diff     = abs(Target - Deg);                     // Calculate difference
  Serial.println("Target = " + String(Target));     // Display the target selected
  
  if (Diff < Deadband) return;                      // Exit if the angle is already close enough to the Target
  if ( (Diff < 180 and Deg < Target) or (Diff > 180 and Deg > Target) )
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
    
  if ( (Diff < 180 and Deg > Target) || (Diff > 180 and Deg < Target))
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
}       

void A() {
  if (Direction == 'F')
    do {
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
      for (byte i = 0; i < Sequence; i++) {
        for (byte j = 0; j < 4 ; j++)  digitalWrite(IN[j], seqC[i][3-j]); 
        delayMicroseconds(Dt);
      }
      Deg = Deg - 2880 / float(Steps);                    // Decrease angle (8 * 360 / Steps = 2880 / Steps)
      if (Deg < 0)     Deg = Deg + 360;
    }
    while (!Serial.available()); 
}
