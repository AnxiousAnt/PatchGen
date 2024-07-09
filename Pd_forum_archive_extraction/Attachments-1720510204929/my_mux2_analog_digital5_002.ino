#define CONTROL0 5
#define CONTROL1 4
#define CONTROL2 3
#define CONTROL3 2

byte muxarray0[33];
byte muxarray1[16];

void setup() {
  pinMode(CONTROL0, OUTPUT);
  pinMode(CONTROL1, OUTPUT);
  pinMode(CONTROL2, OUTPUT);
  pinMode(CONTROL3, OUTPUT);
  Serial.begin(9600);
}

void loop() {
  muxarray0[0] = 0xc0; // denote beginning of data
  byte k = 1;
  for (int i = 0; i < 16; i++) {
    digitalWrite(CONTROL0, (i&15)>>3);
    digitalWrite(CONTROL1, (i&7)>>2);
    digitalWrite(CONTROL2, (i&3)>>1);
    digitalWrite(CONTROL3, (i&1));
    unsigned int mod = analogRead(0);
    muxarray0[k++] = mod & 0x007f;
    muxarray0[k++] = (mod >> 7) & 0x0007; /* I got this from some other sketch. 
    I multiply this by 128 in Pd and then add it to the result of the line of code above this, 
    this way I'm supposed to get a range of 0-1023, cause byte will take only 8bit...
*/
  }
  for (int j = 0; j < 16; j++) {
    digitalWrite(CONTROL0, (j&15)>>3);
    digitalWrite(CONTROL1, (j&7)>>2);
    digitalWrite(CONTROL2, (j&3)>>1);
    digitalWrite(CONTROL3, (j&1));
    muxarray1[j] = digitalRead(A1);
  }
    Serial.write(muxarray0, 33);
    Serial.write(muxarray1, 16);
}
