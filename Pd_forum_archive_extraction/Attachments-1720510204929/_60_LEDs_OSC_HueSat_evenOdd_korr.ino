#include <OSCMessage.h> 
#include <SLIPEncodedSerial.h>
// these two includes are enough for our purpose
// we don't have to import the whole OSC library

#include <FastLED.h>
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
  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
  pinMode(13, OUTPUT);
  SLIPSerial.begin(BAUDRATE);
}


static int fad0; // led 0-14 brightness
static int fad1; // led 15-29 
static int fad2; // led 30-44
static int fad3; // led 45-59

static int fad4; // led 0-14 Hue
static int fad5; // led 15-29 hue
static int fad6; // led 30-44 hue
static int fad7; // led 45-59 hue

static int fad8; // global value

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

      ////test individual OSC receives      
      if (msg.fullMatch("/fad0", 0)){
        // then check if first item is an integer
        if (msg.isInt(0)){
          // use the integer argument for controlling the led 0 
          fad0 = (msg.getInt(0));
        }
        Serial.println (fad0);
      } else if (msg.fullMatch("/fad1", 0)){
        // then check if first item is an integer
        if (msg.isInt(0)){
          // use the integer argument for controlling led 1 
          fad1 = (msg.getInt(0));
        }
        Serial.println (fad1);
      } else if (msg.fullMatch("/fad2", 0)){
        // then check if first item is an integer
        if (msg.isInt(0)){
          // use the integer argument for controlling led 1 
          fad2 = (msg.getInt(0));
        }
        Serial.println (fad2);
      } else if (msg.fullMatch("/fad3", 0)){
        // then check if first item is an integer
        if (msg.isInt(0)){
          // use the integer argument for controlling led 1 
          fad3 = (msg.getInt(0));
        }
        Serial.println (fad3);
      } else if (msg.fullMatch("/fad4", 0)){
        // then check if first item is an integer
        if (msg.isInt(0)){
          // use the integer argument for controlling led 1 
          fad4 = (msg.getInt(0));
        }
        Serial.println (fad4);
      } else if (msg.fullMatch("/fad5", 0)){
        // then check if first item is an integer
        if (msg.isInt(0)){
          // use the integer argument for controlling led 1 
          fad5 = (msg.getInt(0));
        }
        Serial.println (fad5);
      } else if (msg.fullMatch("/fad6", 0)){
        // then check if first item is an integer
        if (msg.isInt(0)){
          // use the integer argument for controlling led 1 
          fad6 = (msg.getInt(0));
        }
        Serial.println (fad6);
      } else if (msg.fullMatch("/fad7", 0)){
        // then check if first item is an integer
        if (msg.isInt(0)){
          // use the integer argument for controlling led 1 
          fad7 = (msg.getInt(0));
        }
        Serial.println (fad7);
      } else if (msg.fullMatch("/fad8", 0)){
        // then check if first item is an integer
        if (msg.isInt(0)){
          // use the integer argument for controlling led 1 
          fad8 = (msg.getInt(0));
        }
        Serial.println (fad8);
      }
    }
  }
  
  //////////  testing single LED on/off::
  {
    if (fad2 >= 1){
      leds[0] = CHSV( fad1, fad0, fad2);
      leds[2] = CHSV( fad1, fad0, fad2);
      leds[4] = CHSV( fad1, fad0, fad2);
      leds[6] = CHSV( fad1, fad0, fad2);
      leds[8] = CHSV( fad1, fad0, fad2);
      leds[10] = CHSV( fad1, fad0, fad2);
      leds[12] = CHSV( fad1, fad0, fad2);
      leds[14] = CHSV( fad1, fad0, fad2);
      leds[16] = CHSV( fad1, fad0, fad2);
      leds[18] = CHSV( fad1, fad0, fad2);
      leds[20] = CHSV( fad1, fad0, fad2);
      leds[22] = CHSV( fad1, fad0, fad2);
      leds[24] = CHSV( fad1, fad0, fad2);
      leds[26] = CHSV( fad1, fad0, fad2);
      leds[28] = CHSV( fad1, fad0, fad2);
    }


  }

  {
    if (fad4 >= 1){
      leds[1] = CHSV( fad3, fad0, fad4);
      leds[3] = CHSV( fad3, fad0, fad4);
      leds[5] = CHSV( fad3, fad0, fad4);
      leds[7] = CHSV( fad3, fad0, fad4);
      leds[9] = CHSV( fad3, fad0, fad4);   
      leds[11] = CHSV( fad3, fad0, fad4);
      leds[13] = CHSV( fad3, fad0, fad4);
      leds[15] = CHSV( fad3, fad0, fad4);
      leds[17] = CHSV( fad3, fad0, fad4);
      leds[19] = CHSV( fad3, fad0, fad4); 
      leds[21] = CHSV( fad3, fad0, fad4);
      leds[23] = CHSV( fad3, fad0, fad4);
      leds[25] = CHSV( fad3, fad0, fad4);
      leds[27] = CHSV( fad3, fad0, fad4);
      leds[29] = CHSV( fad3, fad0, fad4); 
    }

  }

  {
    if (fad6 >= 1){
      leds[30] = CHSV( fad5, fad0, fad6);
      leds[32] = CHSV( fad5, fad0, fad6);
      leds[34] = CHSV( fad5, fad0, fad6);
      leds[36] = CHSV( fad5, fad0, fad6);
      leds[38] = CHSV( fad5, fad0, fad6);
      leds[40] = CHSV( fad5, fad0, fad6);
      leds[42] = CHSV( fad5, fad0, fad6);
      leds[44] = CHSV( fad5, fad0, fad6);
      leds[46] = CHSV( fad5, fad0, fad6);
      leds[48] = CHSV( fad5, fad0, fad6);
      leds[50] = CHSV( fad5, fad0, fad6);
      leds[52] = CHSV( fad5, fad0, fad6);
      leds[54] = CHSV( fad5, fad0, fad6);
      leds[56] = CHSV( fad5, fad0, fad6);
      leds[58] = CHSV( fad5, fad0, fad6);
    }
  }
  {
    if (fad8 >= 1){
      leds[31] = CHSV( fad7, fad0, fad8);
      leds[33] = CHSV( fad7, fad0, fad8);
      leds[35] = CHSV( fad7, fad0, fad8);
      leds[37] = CHSV( fad7, fad0, fad8);   
      leds[39] = CHSV( fad7, fad0, fad8);
      leds[41] = CHSV( fad7, fad0, fad8);
      leds[43] = CHSV( fad7, fad0, fad8);
      leds[45] = CHSV( fad7, fad0, fad8);
      leds[47] = CHSV( fad7, fad0, fad8);   
      leds[49] = CHSV( fad7, fad0, fad8);
      leds[51] = CHSV( fad7, fad0, fad8);
      leds[53] = CHSV( fad7, fad0, fad8);
      leds[55] = CHSV( fad7, fad0, fad8);
      leds[57] = CHSV( fad7, fad0, fad8);   
      leds[59] = CHSV( fad7, fad0, fad8);
    }

  }
  ////////// 
  FastLED.show();
delay(3);
}






