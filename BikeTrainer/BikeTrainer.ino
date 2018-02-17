/**
 * aBikeTrainer - Arduino Bike Trainer
 * 
 * Reads and analyzes data from bike sensors.
 * Monitors wheel & cadence sensor and counts revolutions. 
 * Calculates total time and total distace.
 * Calculates instant velocity and instant cadence rpm. 
 * Calculates average velocity and average cadence rpm.
 * Shows session time on LDC display
 * 
 * Sends wheel & cadence counter values on serial port.
 * 
 * >>> THIS IS A TEST PROTOTYPE <<<
 * 
 * version  0.0.3. 
 * created  12 feb 2018
 * modified 15 feb 2018
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

// ports definition
const int wheelSensorPort   = A0;  // wheel sensor conneted on analog port 0
const int cadenceSensorPort = A1;  // cadence sensor conneted on analog port 1

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
long wheelRevolutionLastTime    = 0;      // last time the wheel sensor was closed
long cadenceRevolutionDuration  = 0;      // time in millis for a cadence revolution
long cadenceRevolutionLastTime  = 0;      // last time the cadence sensor was closed

// data computing
long startTime            = 0;     // trainig starting time
long printTime            = 0;     // last data printing time
float wheelCircumference  = 2096;  // wheel circumference in centimeters

// lcd configuration
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);  // initialize the library with the interface pins
const int left    = 1;  // text lcd left alignment
const int center  = 2;  // text lcd center alignment
const int right   = 3;  // text lcd right alignment


/**
 * Main settings.
 */
void setup() {
  // initializes serial communication  
  Serial.begin(9600);
  // initializes computation variables
  startTime = millis();
  // set up LCD's number of columns and rows
  lcd.begin(16, 2);
  // prints a message on the LCD
  lcdPrint("aBikeTrainer", left, "ver. 0.0.2.", left);  
  delay(2000);
}

/*
 * Main loop.
 * Read wheel and cadence sensor state and count revolutions.
 */
void loop() {
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
  
  // computes session data
  // total time in this session in hour, minute, seconds
  int totalTimeHour = (millis() - startTime) / 3600000;
  int totalTimeMinute = ((millis() - startTime) % 3600000) / 60000;
  int totalTimeSecond = ((millis() - startTime) % 60000) / 1000;
  // total distance amount in this session in km
  float totalDistance = wheelCircumference * wheelCounter / 100000.0;
  // average velocity in km/h, based on distance and total session time
  float averageVelocity = totalDistance / ((millis() - startTime) / 3600000.0);
  // avrage cadence rpm, based on cadence and total session time
  int averageCadenceRpm = (int) (cadenceCounter / ((millis() - startTime) / 60000.0));
  // instant velocity, based on one wheel revolution
  float instantVelocity = (wheelCircumference / 100000.0) / (wheelRevolutionDuration / 3600000.0);
  // instant cadence rpm, based on one pedal revolution
  int instantCadenceRpm = (int) (1.0 / (cadenceRevolutionDuration / 60000.0));

  // prints sensor state
  if (millis() - printTime > printLatency) {
    printTime = millis();
    String totalTime = "";
    for (int i = 0; i < 10 - String(totalTimeHour).length(); i++) {
      totalTime += " ";
    }
    totalTime += totalTimeHour;
    totalTime += ":";
    if (totalTimeMinute <= 9) {
      totalTime += "0";
    }
    totalTime += totalTimeMinute;
    totalTime += ":";
    if (totalTimeSecond <= 9) {
      totalTime += "0";
    }
    totalTime += totalTimeSecond;
    lcdPrint("Time", left, totalTime, right);  
    
   

    // sends data on serial port    
    Serial.print(wheelCounter);
    Serial.print("  ");
    Serial.print(cadenceCounter);
    Serial.print("  ");
    Serial.print(totalTimeHour);
    Serial.print(":");
    Serial.print(totalTimeMinute);
    Serial.print(":");
    Serial.print(totalTimeSecond);
    Serial.print("  ");
    Serial.print(totalDistance);
    Serial.print("  ");
    Serial.print(instantVelocity);
    Serial.print("  ");
    Serial.print(instantCadenceRpm);
    Serial.print("  ");
    Serial.print("  ");
    Serial.print(averageVelocity);
    Serial.print("  ");
    Serial.print(averageCadenceRpm);
    Serial.println("  ");
  }
  
  // wait before next read
  delay(latency);

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

