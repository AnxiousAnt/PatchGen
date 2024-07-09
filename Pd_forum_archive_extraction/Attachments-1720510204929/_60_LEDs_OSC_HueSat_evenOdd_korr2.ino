#include <OSCMessage.h> 
#include <SLIPEncodedSerial.h>
// these two includes are enough for our purpose
// we don't have to import the whole OSC library

// #include <FastLED.h>
#define NUM_LEDS 100
#define DATA_PIN 3


#define BAUDRATE 115200
//#define BAUDRATE 57600


//sets up an array that can be manipulated to set/clear data.
CRGB leds [NUM_LEDS];

SLIPEncodedSerial SLIPSerial(Serial);

int analogValue = 0;
long oldTime = 0;
long newTime = 0;

//tells the library that there is a strand of WS2812 leds on pin 3 and 
//that they will use the "leds" array and that there are 60 leds


void setup()
{
  // FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
  pinMode(13, OUTPUT);
  SLIPSerial.begin(BAUDRATE);
}

static int fad[9] = {
  0, // led 0-14 brightness
  0, // led 0-14 brightness
  0, // led 15-29 
  0, // led 30-44
  0, // led 45-59
  0, // led 0-14 Hue
  0, // led 15-29 hue
  0, // led 30-44 hue
  0  // led 45-59 hue
};




// global value

// define methods????

// end methods 

void loop()
{
  FastLED.clear();

  // look for incoming OSC messages:

  // first check if bytes are available
  while(SLIPSerial.available())
  {
    // create empty OSC Message
    OSCMessage msg;
    // fill OSC message with incoming bytes till you reach end of packet //
    while(!SLIPSerial.endofPacket())
    {
      int size = SLIPSerial.available();
      while (size--)
      {
        msg.fill(SLIPSerial.read());
      }
    }
    ////test individual OSC receives

    char pattern[] = "/fad/0"; // "/fad/N" is better OSC style than "/fadN"
    
    for (int i = 0; i < 9; ++i){
      // change the number in the address to the current index.
      // if you want it to work with numbers that have more than a single digit you have to adapt the code
      pattern[5] = i + '0';
      if (msg.fullMatch(pattern, 0)){
        // then check if first item is an integer
        if (msg.isInt(0)){
          // use the integer argument for controlling the led 0 
          fad[0] = (msg.getInt(0));
        }
        Serial.println (fad[0]);
        break; // jump out of for-loop
      }
    }
  }       
    
  //////////  testing single LED on/off::

  if (fad[2] >= 1){
    for (int i = 0; i < 30; i += 2){
      leds[i] = CHSV( fad[1], fad[0], fad[2]); 
    }
  }
  
  if (fad[4] >= 1){
    for (int i = 1; i < 31; i += 2){
      leds[i] = CHSV( fad[3], fad[0], fad[4]);
    }
  }

  if (fad[6] >= 1){
    for (int i = 30; i < 60; i += 2){
      leds[i] = CHSV( fad[5], fad[0], fad[6]);
    }
  }

  if (fad[8] >= 1){
    for (int i = 33; i < 61; i += 2){
      leds[i] = CHSV( fad[7], fad[0], fad[8]);
    }
  }
  
  ////////// 
  FastLED.show();
  delay(3);
}






