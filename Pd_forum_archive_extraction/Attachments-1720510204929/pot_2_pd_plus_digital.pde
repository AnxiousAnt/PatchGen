/*
This is the code which just reads analog data off the
adc pins and then prints it to serial. 
Connect pots to +5, and GND.  Wipers go adc pins 0,1.
!very rudimentary!

(This was originally written by "Nestor" (see Pd forum)
as "Pot_2_Pd".)

I just tweaked it a bit to add digital inputs.


*/
float val1;
float valDig;
int potPin1 = 0;                                                                                             
int potPin2 = 1;
int digPin2 = 2;
int digPin3 = 3;
int digPin4 = 4;
int digPin5 = 5;
int digPin6 = 6;
int digPin7 = 7;
int digPin8 = 8;
int digPin9 = 9;
void setup(){
  Serial.begin(115200);
  pinMode(potPin1,INPUT);
  pinMode(potPin2,INPUT);
  pinMode(digPin2, INPUT);
  pinMode(digPin3, INPUT);
  pinMode(digPin4, INPUT);
  pinMode(digPin5, INPUT);
  pinMode(digPin6, INPUT);
  pinMode(digPin7, INPUT);  
  pinMode(digPin8, INPUT);
  pinMode(digPin9, INPUT);
}

void loop(){ 

// Analog Pins...
  for (int j=0;j<2;j++) {
    val1=analogRead(j);
    Serial.print(j);
    Serial.println(val1,DEC);
  }
  
// Digital Pins...
  for (int i=2;i<10;i++) {
   valDig=digitalRead(i);
   // "ID" for digital inputs... questionable...
   Serial.print(5,DEC);
   
   Serial.print(i,DEC);
   Serial.println(valDig,0);
  }
  
  
}
