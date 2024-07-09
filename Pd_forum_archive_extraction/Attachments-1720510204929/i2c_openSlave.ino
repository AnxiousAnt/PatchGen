// i2c_openSlave Martin Peach 20200911
// Listens on all addresses for incoming packets.
// A tester for [pii2c] pure-data external for raspis
// Teensy 3.2 to Qwiic connector: black-gnd red-3V blue-sdata-18 yellow-sclock-19
// openSlave Martin Peach 20200911 based on
// -------------------------------------------------------------------------------------------
// Basic Slave
// -------------------------------------------------------------------------------------------
//
// This creates a simple I2C Slave device which will print whatever text string is sent to it.
// It will retain the text string in memory and will send it back to a Master device if 
// requested.  It is intennded to pair with a Master device running the basic_master sketch.
//
// This example code is in the public domain.
//
// -------------------------------------------------------------------------------------------

#include <i2c_t3.h>

// Function prototypes
void receiveEvent(size_t count);
void requestEvent(void);

// Memory
#define MEM_LEN 128
#define N_BUFS 2
typedef char dbuf[MEM_LEN]; 
dbuf databuf[N_BUFS];
volatile uint8_t received[N_BUFS];
volatile uint8_t receivedFrom[N_BUFS];
volatile uint8_t overruns;
volatile uint8_t nosuch;

//
// Setup
//
void setup()
{
    pinMode(LED_BUILTIN,OUTPUT); // LED

    // Setup for Slave mode, all addresses valid, pins 18/19, external pullups, 400kHz
    Wire.begin(I2C_SLAVE, 0x03, 0x77, I2C_PINS_18_19, I2C_PULLUP_EXT, 400000);

    // Data init
    for (byte b = 0; b < N_BUFS; ++b) {
      received[b] = 0;
      memset(databuf[b], 0, sizeof(databuf[b]));
    }

    // register events
    Wire.onReceive(receiveEvent);
    Wire.onRequest(requestEvent);

    Serial.begin(115200);
}

void loop()
{
  // print received data - this is done in main loop to keep time spent in I2C ISR to minimum
  for (byte b = 0; b < N_BUFS; ++b) {
    if(received[b])
    {
      digitalWrite(LED_BUILTIN,HIGH);
      Serial.printf("Slave 0x%02X received: %d bytes\n", receivedFrom[b], received[b]);
      for (int i = 0; i < received[b]; ++i) Serial.printf("0x%02X ", databuf[b][i]);
      Serial.println(".");
      received[b] = 0;
      digitalWrite(LED_BUILTIN,LOW);
    }
  }
  if (0 != overruns) {
    Serial.printf("*** overruns: %d\n", overruns);
    overruns = 0;
  }
  if (0 != nosuch) {
    Serial.printf("*** nosuch: 0x%02X\n", nosuch);
    nosuch = 0;
  }
}

//
// handle Rx Event (incoming I2C data)
//
void receiveEvent(size_t count)
{
  byte b = 0;
  while ((b < N_BUFS) && (0 != received[b])) b++;
  if (b == N_BUFS) {
    ++overruns;
    b = 0;
  }
  Wire.read(databuf[b], count);  // copy Rx data to databuf
  received[b] = count;           // set received flag to count, this triggers print in main loop
  receivedFrom[b] = Wire.getRxAddr();
}

//
// handle Tx Event (outgoing I2C data)
//
void requestEvent(void)
{
  uint8_t requestAddr = Wire.getRxAddr();
  uint8_t b = 0;
  while ((b < N_BUFS)&&(receivedFrom[b] != requestAddr)) b++;
  if (b == N_BUFS) {
    nosuch = requestAddr;
    b = 0;
  }
  Wire.write(databuf[b], MEM_LEN); // fill Tx buffer (send full mem)
}
