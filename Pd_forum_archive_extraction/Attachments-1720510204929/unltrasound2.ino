int trigPin = 13;
int echoPin = 12;
int ledY = 11;
int ledR = 10;

byte transfer_data[3]; // array to transfer byte split in two

void setup(){
 Serial.begin(9600);
pinMode(trigPin,OUTPUT);
pinMode(echoPin,INPUT);
pinMode(ledY,OUTPUT);
pinMode(ledR,OUTPUT);
  
  
}
void loop(){
  transfer_data[0] = 0xc0; // denote beginning of data stream
  int index = 1; // array index offset
  
  long duration,distance;
  digitalWrite(trigPin,LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin,HIGH);
  delayMicroseconds(5);
  digitalWrite(trigPin,LOW);
  duration=pulseIn(echoPin,HIGH);
  distance=(duration/2)/29.1;
  if(distance<20){
    digitalWrite(ledY,HIGH);
    digitalWrite(ledR,LOW);
  }else{
   digitalWrite(ledY,LOW);
  digitalWrite(ledR,HIGH);
  }
  if(distance>=20||distance<=0) distance = 0;
  transfer_data[index++] = distance & 0x007f;
  transfer_data[index++] = distance >> 7;
  Serial.write(tansfer_data, 3);
  }
  delay(500); 
}
