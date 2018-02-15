/**
 * aBikeTrainer - Arduino Bike Trainer
 * 
 * Read and analyze data from bike sensors.
 * Monitor wheel & cadence sensor and count revolutions. 
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
const int sensorLowLevel  = 400;   // under this analog level the sensor in open
const int sensorHighLevel = 600;   // over this analog level the sensor in closed
const int latency         = 10;    // time in millis to wait between sensors state checks

// counters
int wheelCounter      = 0;      // number of wheel turns
int cadenceCounter    = 0;      // number of pedal turns
boolean wheelState    = false;  // state of the wheel sensor - true closed, false open
boolean cadenceState  = false;  // state of the cadence sensor - true closed, false open

// data computing
long startTime      = 0; // trainig starting time


/**
 * Main settings.
 */
void setup() {
  
  // initialize serial comunication  
  Serial.begin(9600);

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
    }
    // sets the sensor closed
    cadenceState = true;
  }

  // send sensor state
  Serial.print(wheelCounter);
  Serial.print("  ");
  Serial.println(cadenceCounter);

  // wait before next read
  delay(latency);

}
