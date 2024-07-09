

int trigPin = 13;
int echoPin = 12;
int ledY = 11;
int ledR = 10;

void setup(){
 Serial.begin(9600);
pinMode(trigPin,OUTPUT);
pinMode(echoPin,INPUT);
pinMode(ledY,OUTPUT);
pinMode(ledR,OUTPUT);
  
  
}
void loop(){
  
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
  if(distance>=20||distance<=0){
   Serial.write("out of Range"); 
  }else{
   Serial.write(distance);
  Serial.write("cm"); 
  }
  delay(500);
  
}
