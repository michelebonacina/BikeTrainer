/**
 * aBikeTrainer - Arduino Bike Trainer
 * 
 * Reads and analyzes data from bike sensors.
 * Monitors wheel & cadence sensor and counts revolutions. 
 * Calculates total time and total distace.
 * Calculates instant velocity and instant cadence rpm 
 * Calculates average velocity and average cadence rpm
 * 
 * Sends wheel & cadence counter values on serial port.
 * 
 * >>> THIS IS A TEST PROTOTYPE <<<
 * 
 * version  0.0.2. 
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

// ports definition
const int wheelSensorPort   = A0; // wheel sensor conneted on analog port 0
const int cadenceSensorPort = A1; // cadence sensor conneted on analog port 1

// sensors configuration
const int sensorLowLevel  = 400; // under this analog level the sensor in open
const int sensorHighLevel = 600; // over this analog level the sensor in closed
const int latency         = 10;  // time in millis to wait between sensors state checks
const int printLatency    = 250; // time in millis to wait between data print updates

// counters
int wheelCounter                = 0;     // number of wheel turns
int cadenceCounter              = 0;     // number of pedal turns
boolean wheelState              = false; // state of the wheel sensor - true closed, false open
boolean cadenceState            = false; // state of the cadence sensor - true closed, false open
long wheelRevolutionDuration    = 0;     // time in millis for a wheel revolution
long wheelRevolutionLastTime    = 0;     // last time the wheel sensor was closed
long cadenceRevolutionDuration  = 0;     // time in millis for a cadence revolution
long cadenceRevolutionLastTime  = 0;     // last time the cadence sensor was closed

// data computing
long startTime            = 0;    // trainig starting time
long printTime            = 0;    // last data printing time
float wheelCircumference  = 2096; // wheel circumference in centimeters


/**
 * Main settings.
 */
void setup() {
  // initialize serial comunication  
  Serial.begin(9600);
  // initialice computation variables
  startTime = millis();
}

/*
 * Main loop.
 * Read wheel and cadence sensor state and count revolutions.
 */
void loop() {
  // read sensors value
  int wheelSensorValue = analogRead(wheelSensorPort);
  int cadenceSensorValue = analogRead(cadenceSensorPort);

  // checks the wheel sensor state
  if (wheelSensorValue < sensorLowLevel) {
    // the wheel sensor is open
    // sets the sensor open
    wheelState = false;
  }
  if (wheelSensorValue > sensorHighLevel) {
    // the wheel sensor is closed
    if (wheelState == false) {
      // before this, the sensor was open
      // the wheel makes another turn
      wheelCounter++;
      // calculate time for one revolution
      wheelRevolutionDuration = millis() - wheelRevolutionLastTime;
      // set revolution starting time
      wheelRevolutionLastTime = millis();
    }
    // sets the sensor closed
    wheelState = true;
  }
  // checks the cadence sensor state
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
      // calculate time for one revolution
      cadenceRevolutionDuration = millis() - cadenceRevolutionLastTime;
      // set revolution starting time
      cadenceRevolutionLastTime = millis();
    }
    // sets the sensor closed
    cadenceState = true;
  }
  
  // compute session data
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

  // send sensor state
  if (millis() - printTime > printLatency) {
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
