#include <AFMotor.h>
#include <Servo.h>

// Define pin constants for easy reference
#define START_BUTTON A1       
#define EMERGENCY_STOP A2     
#define LIMIT_SWITCH A3      
#define IR_PIN A4             // Pin for the infrared (IR) sensor input

// Initialize motor and pump objects on specific motor shield channels
AF_DCMotor motor(1);          
AF_DCMotor pump(2);           

// Create a Servo object to control a servo motor
Servo Servo1;                 

// Constant values to define motor behavior and servo positions
const int MOTOR_SPEED = 255;      // Maximum speed for motor and pump
const int SERVO_OPEN_ANGLE = 90;  
const int SERVO_CLOSE_ANGLE = 45; 
const int DISPENSE_TIME_MS = 10000; // Time in milliseconds for the pump to operate

// Array storing concentration values (used to control the servo timing)
int concentrationValues[5] = {1, 2, 3, 4, 8}; 

// Variable to track the system's running state
bool isRunning = false;

void setup() {
  // Initialize serial communication for debugging
  Serial.begin(9600);
  
  // Attach the servo motor to digital pin 9
  Servo1.attach(9);
  
  // Set the motor and pump speeds to maximum
  motor.setSpeed(MOTOR_SPEED);
  pump.setSpeed(MOTOR_SPEED);
  
  // Ensure both motor and pump are initially stopped
  motor.run(RELEASE);
  pump.run(RELEASE);
  
  // Configure button and sensor pins as input with internal pull-up resistors
  pinMode(START_BUTTON, INPUT_PULLUP);    
  pinMode(EMERGENCY_STOP, INPUT_PULLUP);  
  pinMode(LIMIT_SWITCH, INPUT_PULLUP);    
  pinMode(IR_PIN, INPUT);                 
}

void loop() {
  // Read the current state of the start button, emergency stop, limit switch, and IR sensor
  bool startButtonState = digitalRead(START_BUTTON);
  bool emergencyStopState = digitalRead(EMERGENCY_STOP);
  bool limitSwitchState = digitalRead(LIMIT_SWITCH);
  bool IRval = digitalRead(IR_PIN);

  // If the emergency stop button is pressed, stop all operations immediately
  if (emergencyStopState == LOW) {
    isRunning = false;
    stopAll();  // Call function to halt motors, pump, and servo
    return;     // Exit loop early to prevent further actions
  }

  // If the start button is pressed, set the system to the running state
  if (startButtonState == LOW) {
    isRunning = true;
  }

  // Main control logic for when the system is running
  if (isRunning && limitSwitchState == LOW && IRval == LOW) {
    motor.run(RELEASE);  
    dispense();          

    // Wait here until the IR sensor is no longer triggered (e.g., object has moved away)
    while (digitalRead(IR_PIN) == LOW);
  } else if (isRunning) {
    motor.run(FORWARD);  
  } else {
    motor.run(RELEASE);  
  }
}

void dispense() {
  // Loop through each concentration value to perform a series of dispensing actions
  for (int i = 0; i < 5; i++) {
    // Operate the servo to the open position, wait, then close it
    Serial.println("Operating Servo.");
    Servo1.write(SERVO_OPEN_ANGLE);             
    delay(concentrationValues[i] * 1000);       
    Servo1.write(SERVO_CLOSE_ANGLE);            

    // Activate the pump for a fixed duration to dispense the fluid
    Serial.println("Operating Pump.");
    pump.run(FORWARD);                          
    delay(DISPENSE_TIME_MS);                    
    pump.run(RELEASE);                          
  }

  // After completing all dispensing actions, stop the system
  Serial.println("Dispensing completed.");
  isRunning = false;
}

void stopAll() {
  // Function to stop all moving parts in case of emergency or completion
  motor.run(RELEASE);          // Stop the motor
  pump.run(RELEASE);           // Stop the pump
  Servo1.write(SERVO_CLOSE_ANGLE); // Reset servo to close position
  Serial.println("Emergency Stop Activated. All operations STOPED.");
}
