#include "Tlc5940.h"

const long baudRate = 57600; 
boolean isset = false;
int incomingByte;
int firstByte;
int secondByte;
int solenoidPin;
int velocity;
//long offTimes[8];
//long now;

void  setup()
{
  // start serial communication
  //initialize the TLC5940
  Serial.begin(baudRate);
  Tlc.init();
}


void loop()
{
  // check serial in for incoming messages
  // a complete messages consists of two bytes
  // the bit mask looks as followed:
  //
  // 0 P P P P P P V   1 V V V V V V V
  //
  // 0, 1 :  header of first and second byte
  // P    :  Pin Number 1-64 (currently only 8 are in use)
  // V    :  velocity (0-4095 in 256 steps)
  if(Serial.available() != 0)
  {
    incomingByte = Serial.read();
    
    
    // CODE FOR HANDSHAKE
    if(incomingByte == 255)
    {
      Serial.print("SOL 0 1");
    }
    // END CODE FOR HANDSHAKE
    
    if((incomingByte >> 7) == 0)
    {
      firstByte = incomingByte;
    }
    else
    {
      secondByte = incomingByte;
      // as second bytes is received, the message is complete
      isset = true;
    }
  }
  
  // if a full message is received:
  //   - apply pwm signal (velocity) to respective pin
  
  
  if(isset)
  {
    //here's the bit twiddling stuff that needs to be altered
    //to address up to 44 pins, not just 16
    solenoidPin = firstByte >> 1;
    velocity = ((firstByte % 1) << 7) + (secondByte % 128);
    
    //since the max 'time on' was 1023, map to 4095 (which is the 
    //highest value for each pin on the TLC5940)
    velocity = map(velocity,0,255,0,4095);
    Tlc.set(solenoidPin, velocity);
    Tlc.update();
    isset = false;
  }
}
