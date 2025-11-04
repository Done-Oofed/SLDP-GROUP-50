#include <PulseSensorPlayground.h>
  const int HRSpin = 0;
  const int yes = 2;
  const int no = 3;
  const int misc = 4;
  int iter = 0;
  int HRSout = 0;
  bool wantHBS = false;
  bool notBreak = true;
  bool off = true;
  PulseSensorPlayground HRS;
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);


  HRS.analogInput(0);
  HRS.blinkOnPulse(13);
  HRS.setThreshold(550);

  if(HRS.begin()){
    Serial.println("HBS On");
  }
}
void prompt(){
  //prompt user
    while(notBreak){
    Serial.println("Press Misc to end");
    Serial.println("Do u want heart beat sensor?");
    Serial.println("Press Yes or No");  
    while (digitalRead(yes) == HIGH && digitalRead(no) == HIGH && digitalRead(misc) == HIGH) { //wtf is this empty?
    }
    if(digitalRead(yes) == LOW){
      Serial.println("Yes");
      wantHBS = true;
      break;
    }
    else if (digitalRead(no) == LOW){
      Serial.println("No");
    }
    else{
      while (digitalRead(misc) == LOW) {
      }
      turnoff();
      break;
    }
    while (digitalRead(yes) == LOW || digitalRead(no) == LOW) {
    }
    Serial.println("Do u want glucose monitor?");
    Serial.println("Press Yes or No");
    while (digitalRead(yes) == HIGH && digitalRead(no) == HIGH && digitalRead(misc) == HIGH) {
    }
    if(digitalRead(yes) == LOW){
      Serial.println("Yes");
      break;
    }
    else if (digitalRead(no) == LOW){
      Serial.println("No");
    }
    else{
      while (digitalRead(misc) == LOW) {
      }
      turnoff();
      break;
    }
    while (digitalRead(yes) == LOW || digitalRead(no) == LOW) {
    }
    Serial.println("Do u want blood pressure monitor?");
    Serial.println("Press Yes or No");
    while (digitalRead(yes) == HIGH && digitalRead(no) == HIGH && digitalRead(misc) == HIGH) {
    }
    if(digitalRead(yes) == LOW){
      Serial.println("Yes");
      break;
    }
    else if (digitalRead(no) == LOW){
      Serial.println("No");
    }
    else{
      while (digitalRead(misc) == LOW) {
      }
      turnoff();
      break;
    }
    while (digitalRead(yes) == LOW || digitalRead(no) == LOW) { // while pressed, puase
    }
    Serial.println("Do u want thermometer?");
    Serial.println("Press Yes or No");
    while (digitalRead(yes) == HIGH && digitalRead(no) == HIGH && digitalRead(misc) == HIGH) {
    }
    if(digitalRead(yes) == LOW){
      Serial.println("Yes");
      break;
    }
    else if (digitalRead(no) == LOW){
      Serial.println("No");
    }
    else{
      while (digitalRead(misc) == LOW) {
      }
      turnoff();
      break;
    }
    while (digitalRead(yes) == LOW || digitalRead(no) == LOW) {
    }
    Serial.println("Goodbye!");
    turnoff();
    notBreak=false;
  }
}
void turnoff(){
  Serial.println("Turn LCD off");
  off=true;
}
void turnon(){
  Serial.println("Turn LCD on");
  notBreak=true;
  off=false;
}
void loop() {
  // put your main code here, to run repeatedly:
  if(wantHBS){
    if(HRS.sawStartOfBeat()){
      HRSout = HRS.getBeatsPerMinute();
      Serial.println("Start beat");
      Serial.println(HRSout);
    }
  }
  while(off){
    Serial.println("LCD off");
    while (digitalRead(misc) == HIGH) {
    }
    Serial.println("Button pressed");
    turnon();
    while (digitalRead(misc) == LOW) {
    }
  }
  if(!wantHBS){
    prompt();
  }
  while (digitalRead(yes) == LOW) {
  }
  delay(200);
}



