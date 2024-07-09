#include <SLIPEncodedSerial.h>

#define BAUDRATE 57600

SLIPEncodedSerial SLIPSerial(Serial);

void setup() {
  
  SLIPSerial.begin(BAUDRATE);
  pinMode(2, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(7, OUTPUT);
  pinMode(8, OUTPUT);
  
}

void loop() {
   
   //check if bytes are available
   while(SLIPSerial.available()){
     // kind of the address
     int index = 0;
     // the value
     int value = 0;
     // counter to switch between index and value
     int count = 0;
     while(!SLIPSerial.endofPacket()){
       int size = SLIPSerial.available();
       while (size--){
         int data = SLIPSerial.read();
         if (count == 0){
           index = data;
         }
         if (count == 1){
           value = data;
         }
         count++;
         count = count % 2;
       }
     }
     
     switch(index){
         case 0:
         {
           analogWrite(5, value);
           break;    
         }
         case 1:
         {
           analogWrite(6, value);
           break;    
         }
         case 2:
         {
           analogWrite(9, value);
           break;    
         }
         case 3:
         {
           analogWrite(10, value);
           break;    
         }
         case 4:
         {
           digitalWrite(2, (value > 0));
           break;    
         }
         case 5:
         {
           digitalWrite(4, (value > 0));
           break;   
         }
         case 6:
         {
           digitalWrite(7, (value > 0));
           break;    
         }
         case 7:
         {
           digitalWrite(8, (value > 0));
           break;    
         }    
      }
   }    
}


 
 
    
