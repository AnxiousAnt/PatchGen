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
  }       
    
  //////////  testing single LED on/off::
  {
    if (fad[2] >= 1){
      leds[0] = CHSV( fad[1], fad[0], fad[2]);
      leds[2] = CHSV( fad[1], fad[0], fad[2]);
      leds[4] = CHSV( fad[1], fad[0], fad[2]);
      leds[6] = CHSV( fad[1], fad[0], fad[2]);
      leds[8] = CHSV( fad[1], fad[0], fad[2]);
      leds[10] = CHSV( fad[1], fad[0], fad[2]);
      leds[12] = CHSV( fad[1], fad[0], fad[2]);
      leds[14] = CHSV( fad[1], fad[0], fad[2]);
      leds[16] = CHSV( fad[1], fad[0], fad[2]);
      leds[18] = CHSV( fad[1], fad[0], fad[2]);
      leds[20] = CHSV( fad[1], fad[0], fad[2]);
      leds[22] = CHSV( fad[1], fad[0], fad[2]);
      leds[24] = CHSV( fad[1], fad[0], fad[2]);
      leds[26] = CHSV( fad[1], fad[0], fad[2]);
      leds[28] = CHSV( fad[1], fad[0], fad[2]);
    }


  }

  {
    if (fad[4] >= 1){
      leds[1] = CHSV( fad[3], fad[0], fad[4]);
      leds[3] = CHSV( fad[3], fad[0], fad[4]);
      leds[5] = CHSV( fad[3], fad[0], fad[4]);
      leds[7] = CHSV( fad[3], fad[0], fad[4]);
      leds[9] = CHSV( fad[3], fad[0], fad[4]);   
      leds[11] = CHSV( fad[3], fad[0], fad[4]);
      leds[13] = CHSV( fad[3], fad[0], fad[4]);
      leds[15] = CHSV( fad[3], fad[0], fad[4]);
      leds[17] = CHSV( fad[3], fad[0], fad[4]);
      leds[19] = CHSV( fad[3], fad[0], fad[4]); 
      leds[21] = CHSV( fad[3], fad[0], fad[4]);
      leds[23] = CHSV( fad[3], fad[0], fad[4]);
      leds[25] = CHSV( fad[3], fad[0], fad[4]);
      leds[27] = CHSV( fad[3], fad[0], fad[4]);
      leds[29] = CHSV( fad[3], fad[0], fad[4]); 
    }

  }

  {
    if (fad[6] >= 1){
      leds[30] = CHSV( fad[5], fad[0], fad[6]);
      leds[32] = CHSV( fad[5], fad[0], fad[6]);
      leds[34] = CHSV( fad[5], fad[0], fad[6]);
      leds[36] = CHSV( fad[5], fad[0], fad[6]);
      leds[38] = CHSV( fad[5], fad[0], fad[6]);
      leds[40] = CHSV( fad[5], fad[0], fad[6]);
      leds[42] = CHSV( fad[5], fad[0], fad[6]);
      leds[44] = CHSV( fad[5], fad[0], fad[6]);
      leds[46] = CHSV( fad[5], fad[0], fad[6]);
      leds[48] = CHSV( fad[5], fad[0], fad[6]);
      leds[50] = CHSV( fad[5], fad[0], fad[6]);
      leds[52] = CHSV( fad[5], fad[0], fad[6]);
      leds[54] = CHSV( fad[5], fad[0], fad[6]);
      leds[56] = CHSV( fad[5], fad[0], fad[6]);
      leds[58] = CHSV( fad[5], fad[0], fad[6]);
    }
  }
  {
    if (fad[8] >= 1){
      leds[31] = CHSV( fad[7], fad[0], fad[8]);
      leds[33] = CHSV( fad[7], fad[0], fad[8]);
      leds[35] = CHSV( fad[7], fad[0], fad[8]);
      leds[37] = CHSV( fad[7], fad[0], fad[8]);   
      leds[39] = CHSV( fad[7], fad[0], fad[8]);
      leds[41] = CHSV( fad[7], fad[0], fad[8]);
      leds[43] = CHSV( fad[7], fad[0], fad[8]);
      leds[45] = CHSV( fad[7], fad[0], fad[8]);
      leds[47] = CHSV( fad[7], fad[0], fad[8]);   
      leds[49] = CHSV( fad[7], fad[0], fad[8]);
      leds[51] = CHSV( fad[7], fad[0], fad[8]);
      leds[53] = CHSV( fad[7], fad[0], fad[8]);
      leds[55] = CHSV( fad[7], fad[0], fad[8]);
      leds[57] = CHSV( fad[7], fad[0], fad[8]);   
      leds[59] = CHSV( fad[7], fad[0], fad[8]);
    }

  }
  
  ////////// 
  FastLED.show();
  delay(3);
}






