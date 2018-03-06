/**
 * aBikeTrainer - Arduino Bike Trainer
 * 
 * Reads and analyzes data from bike sensors.
 * Monitors wheel & cadence sensor and counts revolutions. 
 * Calculates total time and total distace.
 * Calculates instant velocity and instant cadence rpm. 
 * Calculates average velocity and average cadence rpm.
 * Calculates max velocity and max cadence rpm.
 * Shows data on LCD, changing using next and previuos buttoms.
 * Starts and stop session using set button. Start reset session data.
 * 
 * >>> THIS IS A TEST PROTOTYPE <<<
 * 
 * version  0.0.5. 
 * created  12 feb 2018
 * modified 06 mar 2018
 * by michele bonacina
 * 
 * 
 * 
 * Copyright 2018 Michele Bonacina
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * 
 */
// include the library code:
#include <LiquidCrystal.h>  // LCD library

// ports configuration
const int wheelSensorPort   = A0;  // wheel sensor conneted on analog port 0
const int cadenceSensorPort = A1;  // cadence sensor conneted on analog port 1
const int setButton         = 8;   // set button connected to digital port 8
const int prevButton        = 9;   // previous button connected to digital port 9
const int nextButton        = 10;  // next button connected to digital port 10

// lcd configuration
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);  // initialize the library with the interface pins
const int left            = 1;  // text lcd left alignment
const int center          = 2;  // text lcd center alignment
const int right           = 3;  // text lcd right alignment
const int numSessionData  = 10; // number of session data

// sensors configuration
const int sensorLowLevel  = 400;  // under this analog level the sensor in open
const int sensorHighLevel = 600;  // over this analog level the sensor in closed
const int latency         = 10;   // time in millis to wait between sensors state checks
const int printLatency    = 250;  // time in millis to wait between data print updates

// counters
int wheelCounter                = 0;      // number of wheel turns
int cadenceCounter              = 0;      // number of pedal turns
boolean wheelState              = false;  // state of the wheel sensor - true closed, false open
boolean cadenceState            = false;  // state of the cadence sensor - true closed, false open
long wheelRevolutionDuration    = 0;      // time in millis for a wheel revolution
long wheelRevolutionLastTime    = 0;      // last time the wheel sensor has been closed
long cadenceRevolutionDuration  = 0;      // time in millis for a cadence revolution
long cadenceRevolutionLastTime  = 0;      // last time the cadence sensor has been closed
int currentLcdData              = 0;      // the index of the current data to be printed on LCD display
float maxVelocity               = 0;      // the max velocity obtained during the session
float maxCadenceRpm             = 0;      // the max cadence rpm obtained during the session

// data computing
long startTime                        = 0;     // trainig starting time
long totalTime                        = 0;     // trainig total time
long printTime                        = 0;     // last data printing time
boolean isRunning                     = false; // session running status
int setButtonValue                    = LOW;   // current set button value
int nextButtonValue                   = LOW;   // current next button value
int prevButtonValue                   = LOW;   // current previous button value
float wheelCircumference              = 2089;  // wheel circumference in millimeters
String sessionData[numSessionData][2];         // session data


/**
 * Main settings.
 */
void setup() {
  // sets up LCD's number of columns and rows
  lcd.begin(16, 2);
  // sets up session data
  sessionData[0][0] = "Wheel #";
  sessionData[1][0] = "Cadence #";
  sessionData[2][0] = "Time";
  sessionData[3][0] = "Distance";
  sessionData[4][0] = "Avg Velocity";
  sessionData[5][0] = "Avg Cadence RPM";
  sessionData[6][0] = "Velocity";
  sessionData[7][0] = "Cadence RPM";
  sessionData[8][0] = "Max Velocity";
  sessionData[9][0] = "Max Cadence RPM";
  // sets up buttons
  pinMode(setButton, INPUT);
  pinMode(nextButton, INPUT);
  pinMode(prevButton, INPUT);
  // prints a message on the LCD
  lcdPrint("aBikeTrainer", left, "ver. 0.0.5.", left);  
  delay(2000);
}


/*
 * Main loop.
 * Reads wheel and cadence sensor state and counts revolutions.
 * Checks buttons state and start/stop session (set button) o change visualization (previous/next buttons)
 * Computes session data if it's time to show them.
 * Prints session data on LCD.
 */
void loop() {
  // checks sensors
  sensorsCheck();
  // checks buttons
  buttonsCheck();
  // prints sensor state
  if (millis() - printTime > printLatency) {
    // resets print timestamp
    printTime = millis();
    // computes data
    dataCalculation();
    // prints data on LCD
    lcdPrint(sessionData[currentLcdData][0], left, sessionData[currentLcdData][1], right);  
  }
  // wait before next read
  delay(latency);
}

/**
 * Set button has been pressed.
 * If session is running stop it.
 * If session is stopped, reset counter and start it.
 */
void setButtonPressed() {
  // checks session state
  if (!isRunning) {
    // session is stopped
    // store new session starting time
    startTime = millis();  
    // reset wheel and cadence counters
    wheelCounter = 0;
    cadenceCounter = 0;
    maxVelocity = 0;
    maxCadenceRpm = 0;
  }
  // if session is started, stop it
  // if session is stopped, start it
  isRunning = !isRunning;
}

/**
 * Next button has been pressed.
 * Switch to next session data, restarting from first after the last.
 */
void nextButtonPressed() {
    // switches to next data
    currentLcdData++;
    // checks data position
    if (currentLcdData > numSessionData - 1) {
      // it's after the last
      // switch to first
      currentLcdData = 0;
    }
}

/**
 * Previous button has been pressed.
 * Switch to previous session data, restarting from last before the first..
 */
void prevButtonPressed() {
    // switches to previous data
    currentLcdData--;
    // check data position
    if (currentLcdData < 0) {
      // it's before the first
      // switch to the last
      currentLcdData = numSessionData - 1;
    }
}

/**
 * Computes session data.
 * Calculates and store session wheel revolutions, cadence revolutions, total time,
 * total distance, average velocity, average cadence rpm, instant velocity, instant cadence rpm.
 */
void dataCalculation() {
  // computes session data
  // total wheel and cadence revolutions
  sessionData[0][1] = String(wheelCounter);
  sessionData[1][1] = String(cadenceCounter);
  // total time in this session in hour, minute, seconds
  if (isRunning) {
    totalTime = millis() - startTime;
  }
  String totalTimeString = "";
  totalTimeString += totalTime / 3600000;
  totalTimeString += ":";
  if ((totalTime % 3600000) / 60000 <= 9) {
    totalTimeString += "0";
  }
  totalTimeString += (totalTime % 3600000) / 60000;
  totalTimeString += ":";
  if ((totalTime % 60000) / 1000 <= 9) {
    totalTimeString += "0";
  }
  totalTimeString += (totalTime % 60000) / 1000;
  sessionData[2][1] = String(totalTimeString);
  // total distance amount in this session in km
  float totalDistance = wheelCircumference * wheelCounter / 1000000.0;
  sessionData[3][1] = String(totalDistance);
  // average velocity in km/h, based on distance and total session time
  float averageVelocity = totalDistance / ((millis() - startTime) / 3600000.0);
  sessionData[4][1] = String(averageVelocity);
  // avrage cadence rpm, based on cadence and total session time
  int averageCadenceRpm = (int) (cadenceCounter / ((millis() - startTime) / 60000.0));
  sessionData[5][1] = String(averageCadenceRpm);
  // instant velocity, based on one wheel revolution
  float instantVelocity = (wheelCircumference / 1000000.0) / (wheelRevolutionDuration / 3600000.0);
  sessionData[6][1] = String(instantVelocity);
  // instant cadence rpm, based on one pedal revolution
  int instantCadenceRpm = (int) (1.0 / (cadenceRevolutionDuration / 60000.0));
  sessionData[7][1] = String(instantCadenceRpm);  
  // max velocity
  if (instantVelocity > maxVelocity) {
    // new max velocity
    maxVelocity = instantVelocity;
  }
  sessionData[8][1] = String(maxVelocity);    
  // max cadence rpm
  if (instantCadenceRpm > maxCadenceRpm) {
    // new max cadence rpm
    maxCadenceRpm = instantCadenceRpm;
  }
  sessionData[9][1] = String(maxCadenceRpm);    
}

/**
 * Checks buttons status.
 * Checks if the buttons has been pressed. It consider a button pressed when it it's released.
 * If pressed, calls the specific button-pressed function.
 */
void buttonsCheck() {
  // reads buttons value
  // set button state
  if (digitalRead(setButton) == LOW && setButtonValue == HIGH) {
    // set button has been released
    setButtonPressed();
  }
  setButtonValue = digitalRead(setButton);
  // next button state
  if (digitalRead(nextButton) == LOW && nextButtonValue == HIGH) {
    // set button has been released
    nextButtonPressed();
  }
  nextButtonValue = digitalRead(nextButton);
  // previous button state
  if (digitalRead(prevButton) == LOW && prevButtonValue == HIGH) {
    // previous button has been released
    prevButtonPressed();
  }
  prevButtonValue = digitalRead(prevButton);
}

/**
 * Checks sensor status.
 * Checks if magnet is passed over the sensor closing the switch.
 * The switch iss close if analog value read if over the min close level.
 * If the switch turns from closed to opened, increments correspondig counter, 
 * calculates revolution time and stores last revolution timestamp.
 * Stores the sensor state.
 */
void sensorsCheck() {
  // reads sensors value
  int wheelSensorValue = analogRead(wheelSensorPort);
  int cadenceSensorValue = analogRead(cadenceSensorPort);
  // checks wheel sensor state
  if (wheelSensorValue < sensorLowLevel) {
    // the wheel sensor is open
    // sets sensor open
    wheelState = false;
  }
  if (wheelSensorValue > sensorHighLevel) {
    // the wheel sensor is closed
    if (wheelState == false) {
      // before this, the sensor was open
      // the wheel makes another turn
      wheelCounter++;
      // calculates time for one revolution
      wheelRevolutionDuration = millis() - wheelRevolutionLastTime;
      // sets revolution starting time
      wheelRevolutionLastTime = millis();
    }
    // sets sensor closed
    wheelState = true;
  }
  // checks cadence sensor state
  if (cadenceSensorValue < sensorLowLevel) {
    // the cadence sensor is open
    // sets the sensor open
    cadenceState = false;
  }
  if (cadenceSensorValue > sensorHighLevel) {
    // the cadence sensor is closed
    if (cadenceState == false) {
      // before this, the sensor was open
      // the pedal makes another turn
      cadenceCounter++;
      // calculates time for one revolution
      cadenceRevolutionDuration = millis() - cadenceRevolutionLastTime;
      // sets revolution starting time
      cadenceRevolutionLastTime = millis();
    }
    // sets sensor closed
    cadenceState = true;
  }  
}


/**
 * Print data on a 16x2 LCD.
 * Print two lines on the 16x2 LCD with specified alignment.
 * Parameters:
 *   - firstLine: text to be printed on the first line
 *   - firstLineAligment: first line alignment (left, center, right)
 *   - secondLine: text to be printed on the second line
 *   - secondLineAligment: second line alignment (left, center, right)
 */
void lcdPrint(String firstLine, int firstLineAlignment, String secondLine, int secondLineAlignment) {
  // initializes first line text
  String firstLineText = "";
  // checks first line aligment type
  if (firstLineAlignment == left) {
    // left alignment
    // text before
    firstLineText += firstLine;
    // spaces after
    for (int i = 0; i < 16 - firstLine.length(); i++) {
      firstLineText += " ";
    }
  } else if (firstLineAlignment == right) {
    // right aligmnent
    // spaces before
    for (int i = 0; i < 16 - firstLine.length(); i++) {
      firstLineText += " ";
    }
    // text after
    firstLineText += firstLine;
  } else if (firstLineAlignment == center) {
    // center aligmnent
    // half spaces before
    for (int i = 0; i < (16 - firstLine.length()) / 2; i++) {
      firstLineText += " ";
    }
    // text in center
    firstLineText += firstLine;
    // half spaces after
    for (int i = 0; i < 16 - firstLineText.length(); i++) {
      firstLineText += " ";
    }
  }
  // initializes second line text
  String secondLineText = "";
  // checks second line alignment type
  if (secondLineAlignment == left) {
    // left alignment
    // text before
    secondLineText += secondLine;
    // spaces after
    for (int i = 0; i < 16 - secondLine.length(); i++) {
      secondLineText += " ";
    }
  } else if (secondLineAlignment == right) {
    // right alignment
    // spaces before
    for (int i = 0; i < 16 - secondLine.length(); i++) {
      secondLineText += " ";
    }
    // text after
    secondLineText += secondLine;
  } else if (secondLineAlignment == center) {
    // center salignment
    // half spaces before
    for (int i = 0; i < (16 - secondLine.length()) / 2; i++) {
      secondLineText += " ";
    }
    // text in center
    secondLineText += secondLine;
    // half spaces after
    for (int i = 0; i < 16 - secondLineText.length(); i++) {
      secondLineText += " ";
    }
  }
  // prints lines on lcd
  lcd.setCursor(0, 0);
  lcd.print(firstLineText);
  lcd.setCursor(0, 1);
  lcd.print(secondLineText);
}

