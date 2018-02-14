/**
 * aBikeTrainer - Arduino Bike Trainer
 * 
 * Read and analyze data from bike sensors.
 * Read analog sensor value, the first from wheel sensor and the second from 
 * cadence sensor. Once readed send values throw serial port in sequence: 
 * sync value (-1), wheel sensor value, cadence sensor value. Sensor values are
 * in range between 0 and 1023.
 * 
 * >>> THIS IS A TEST PROTOTYPE <<<
 * 
 * version  0.0.1. 
 * created  12 feb 2018
 * modified 14 feb 2018
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

/**
 * Main settings.
 */
void setup() {
  
  // initialize serial comunication  
  Serial.begin(9600);

}

/*
 * Main loop
 */
void loop() {
  // read sensors value
  int wheelSensorValue = analogRead(A0);
  int cadenceSensorValue = analogRead(A1);

  // send sync value 
  Serial.println(-1);
  // send sensor value
  Serial.println(wheelSensorValue);
  Serial.println(cadenceSensorValue);

  // wait before next read
  delay(500);

}
