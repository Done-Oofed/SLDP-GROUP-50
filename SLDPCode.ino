#include <PulseSensorPlayground.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <MAX30105.h> //GLC library
#include "heartRate.h"

#include <Adafruit_MLX90614.h> //Temp library

  const int HRSpin = 0;
  const int yes = 2;
  const int no = 3;
  const int misc = 5;
  int iter = 0;
  int HRSout = 0;
  bool wantHBS = false;
  bool wantTemp=false;
  bool notBreak = true;
  bool off = true;
  MAX30105 GLC;
  float temp;
  float IR;
  int HB;
  LiquidCrystal_I2C lcd(0x27,20,4);
  Adafruit_MLX90614 mlx = Adafruit_MLX90614();
  const byte RATE_SIZE = 4; //Increase this for more averaging. 4 is good.
  byte rates[RATE_SIZE]; //Array of heart rates
  byte rateSpot = 0;
  long lastBeat = 0; //Time at which the last beat occurred

  float beatsPerMinute;
  int beatAvg;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  lcd.init();
  //Initialize mlx sensor
  if (!mlx.begin()) {
    Serial.println("MLX90614 was not found. Please check wiring/power.");
    while (1);
  };
  //initialize HBS sensor (aka GLC)
  if (!GLC.begin(Wire, I2C_SPEED_FAST)) { // Start communication using fast I2C speed
    Serial.println("MAX30102 was not found. Please check wiring/power. ");
    while (1); // Infinite loop to halt further execution if sensor is not found
  }
  GLC.setup(); // Configure sensor with default settings for heart rate monitoring
  GLC.setPulseAmplitudeRed(0x0A); // Set the red LED pulse amplitude (intensity) to a low value as an indicator
  GLC.setPulseAmplitudeGreen(0);
}
//Dont worry about prompt(), it should work correctly and only changes the wantHBS and wantTemp if user presses yes
void prompt(){
    while(notBreak){
    Serial.println("Press Misc to end");
    Serial.println("Do u want heart beat sensor?");
    Serial.println("Press Yes or No");
    while (digitalRead(yes) == HIGH && digitalRead(no) == HIGH && digitalRead(misc) == HIGH) {
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
    while (digitalRead(yes) == LOW || digitalRead(no) == LOW) {
    }
    Serial.println("Do u want thermometer?");
    Serial.println("Press Yes or No");
    while (digitalRead(yes) == HIGH && digitalRead(no) == HIGH && digitalRead(misc) == HIGH) {
    }
    if(digitalRead(yes) == LOW){
      wantTemp=true;
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
  lcd.noBacklight();
}
void HRS(){
  iter=0;
  while(iter<100){ //Change to how many times we want to loop through the sensing, then print out final average
    if(checkForBeat(irValue)==true){
      long delta = millis() - lastBeat;
      lastBeat = millis();

      beatsPerMinute = 60 / (delta / 1000.0);

      if (beatsPerMinute < 255 && beatsPerMinute > 20)
      {
        rates[rateSpot++] = (byte)beatsPerMinute;
        rateSpot %= RATE_SIZE;
        beatAvg = 0;
        for (byte x = 0 ; x < RATE_SIZE ; x++)
          beatAvg += rates[x];
        beatAvg /= RATE_SIZE;
      }
    }
    iter++;
  }
  Serial.print("IR=");
  Serial.print(irValue);
  Serial.print(", BPM=");
  Serial.print(beatsPerMinute);
  Serial.print(", Avg BPM=");
  Serial.print(beatAvg);
  wantHRS=false;
}
void temp(){
  //do same loop of iter then in loop code to find temp
  wantTemp=false;
}
void turnon(){
  Serial.println("Turn LCD on");
  lcd.backlight();
  notBreak=true;
  off=false;
}
void loop() {
  if(wantHBS){
    HRS();
  }
  if(wantTEMP){
    temp();
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
  if(!wantHBS&&!wantTemp){
    prompt();
  }
  delay(200);
}



