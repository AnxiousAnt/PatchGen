

int trigPin = 13;
int echoPin = 12;
int ledY = 11;
int ledR = 10;

void setup(){
 Serial begin(9600);
pinMode(trigPin,OUTPUT);
pinMode(echoPin,INPUT);
pinMode(ledY,OUTPUT);
pinMode(ledR,OUTPUT);
  
  
}
void loop(){
  
  long duration,didtance;
  digitalWrite(trigPin,LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin,HIGH);
  delayMicroseconds(5);
  digitalWrite(trigPin,LOW);
  duration=pulseIn(echoPin,HIGH);
  distance=(duration/2)/29.1;
  if(distance<4){
    digitalWrite(ledY,HIGH);
    digitalWrite(ledR,LOW);
  }else{
   digitalWrite(ledY,LOW):
  digitalWrite(ledR,HIGH);
  }
  if(distance>=200||distance<=0){
   Serial.println("out of Range"); 
  }else{
   Serial.print(distance);
  Serial.println("cm"); 
  }
  delay(500);
  
}
