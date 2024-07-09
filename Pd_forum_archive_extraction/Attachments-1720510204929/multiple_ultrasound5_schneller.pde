// script for 5 Ultrasound Sensors

 
int ultraSoundSignal1 = 2;             // Ultrasound signal pin
int ultraSoundSignal2 = 3;             // Ultrasound signal pin
int ultraSoundSignal3 = 5;             // Ultrasound signal pin
int ultraSoundSignal4 = 7;
int ultraSoundSignal5 = 8;   
int val = 0;
int ultrasoundValue1 = 0; 
int ultrasoundValue2 = 0; 
int ultrasoundValue3 = 0; 
int ultrasoundValue4 = 0;
int ultrasoundValue5 = 0; 
int timecount = 0;  // Echo counter                   

void setup() {
  beginSerial(115200);                  // Sets the baud rate to 115200
}


int getSensorValue(int sUltraSoundSignal) {
  timecount = 0;
  val = 0;
  pinMode(sUltraSoundSignal, OUTPUT);     // Switch signalpin to output

  /* Send low-high-low pulse to activate the trigger pulse of the sensor
   * -------------------------------------------------------------------
   */
  digitalWrite(sUltraSoundSignal, LOW);   // Send low pulse 
  delayMicroseconds(2);                            // Wait for 2 microseconds
  digitalWrite(sUltraSoundSignal, HIGH);  // Send high pulse 
  delayMicroseconds(5);                            // Wait for 5 microseconds
  digitalWrite(sUltraSoundSignal, LOW);   // Holdoff


  /* Listening for echo pulse
   * -------------------------------------------------------------------
   */
  pinMode(sUltraSoundSignal, INPUT);      // Switch signalpin to input
  val = digitalRead(sUltraSoundSignal);   // Append signal value to val

  while(val == LOW) {                    // Loop until pin reads a high value
    val = digitalRead(sUltraSoundSignal);
  }

  while(val == HIGH) {                   // Loop until pin reads a high value
    val = digitalRead(sUltraSoundSignal);
    timecount = timecount +1;            // Count echo pulse time
  }
  return timecount;
}

 
void loop() {
  /* Writing out values to the serial port
   * -------------------------------------------------------------------
   */
  ultrasoundValue1 = getSensorValue(ultraSoundSignal1);          // Append echo pulse time to ultrasoundValue
  delay(6);
  ultrasoundValue2 = getSensorValue(ultraSoundSignal2);          // Append echo pulse time to ultrasoundValue
  delay(7);
  ultrasoundValue3 = getSensorValue(ultraSoundSignal3);          // Append echo pulse time to ultrasoundValue
  delay(18);
  ultrasoundValue4 = getSensorValue(ultraSoundSignal4);          // Append echo pulse time to ultrasoundValue
  delay(9);
  ultrasoundValue5 = getSensorValue(ultraSoundSignal5);          // Append echo pulse time to ultrasoundValue
  delay(10);
  
  printByte('A');
  printInteger(ultrasoundValue1);
  printByte(10);
  delay(6);
  
  printByte('B');
  printInteger(ultrasoundValue2);
  printByte(10);
  delay(7);
  
  printByte('C');
  printInteger(ultrasoundValue3);
  printByte(10);
  delay(8);
  
  printByte('D');
  printInteger(ultrasoundValue4);
  printByte(10);
  delay(9);
  
  printByte('E');
  printInteger(ultrasoundValue5);
  printByte(10);
  delay(10);
  
  
}

