#include <PulseSensorPlayground.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <MAX30105.h> //GLC library
#include "heartRate.h"
#include <Adafruit_MLX90614.h> //Temp library
  const char* eyes[] = {
    "=== ===",  // two eyes, left/right
    "--- ---",
    "-   -",
    "o   o",
    "O   O",
    "-   -",
    "--- ---",
    "-   -",
    "o   o",
    "O   O"
};

const char* mouths[] = {
    "___",   // solid line, powering up
    "---",   // simple neutral line
    "_ _",   // small gap
    ". .",   // dots as LEDs
    "_._",   // separated parts
    "...",   // idle
    "_..",   // starting up
    "-.-",   // blinking line
    "._.",   // symmetrical idle
    "---"    // stable line
};
  long irValue;
  const int HRSpin = 0;
  const int yes = 2;
  const int no = 3;
  const int misc = 5;
  int iter = 0;
  int HRSout = 0;
  bool wantHRS = false;
  bool wantTEMP = false;
  bool notBreak = true;
  bool off = true;
  MAX30105 GLC;
  float temp;
  float IR;
  int HB;
  float beatsPerMinute;
  int beatAvg;
  LiquidCrystal_I2C lcd(0x27,20,4);
  byte rates[6]; //Array of heart rates
  byte rateSpot;
  long lastBeat;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  lcd.init();
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
    lcd.clear();
    lcd.print("Do u want");
    lcd.setCursor(0, 1);
    lcd.print("heart beat?");
    lcd.setCursor(0, 2);
    lcd.print("Press Yes or No");
    lcd.setCursor(0, 3);
    lcd.print("Press Misc to end");
    lcd.setCursor(0, 0);
    while (digitalRead(yes) == HIGH && digitalRead(no) == HIGH && digitalRead(misc) == HIGH) {
    }
    if(digitalRead(yes) == LOW){
      Serial.println("Yes");
      wantHRS = true;
      break;
    }
    else if (digitalRead(no) == LOW){
      Serial.println("No");
    }
    else{
      while (digitalRead(misc) == LOW) {
      }
      lcd.clear();
      lcd.setCursor(0, 1);
      lcd.print("Goodbye!");
      lcd.setCursor(0, 0);
      delay(2000);
      turnoff();
      break;
    }
    while (digitalRead(yes) == LOW || digitalRead(no) == LOW) {
    }
    lcd.clear();
    lcd.print("Do u want");
    lcd.setCursor(0, 1);
    lcd.print("glucose?");
    lcd.setCursor(0, 2);
    lcd.print("Press Yes or No");
    lcd.setCursor(0, 3);
    lcd.print("Press Misc to end");
    lcd.setCursor(0, 0);
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
      lcd.clear();
      lcd.setCursor(0, 1);
      lcd.print("Goodbye!");
      lcd.setCursor(0, 0);
      delay(2000);
      turnoff();
      break;
    }
    while (digitalRead(yes) == LOW || digitalRead(no) == LOW) {
    }
    lcd.clear();
    lcd.print("Do u want");
    lcd.setCursor(0, 1);
    lcd.print("blood pressure?");
    lcd.setCursor(0, 2);
    lcd.print("Press Yes or No");
    lcd.setCursor(0, 3);
    lcd.print("Press Misc to end");
    lcd.setCursor(0, 0);
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
      lcd.clear();
      lcd.setCursor(0, 1);
      lcd.print("Goodbye!");
      lcd.setCursor(0, 0);
      delay(2000);
      turnoff();
      break;
    }
    while (digitalRead(yes) == LOW || digitalRead(no) == LOW) {
    }
    lcd.clear();
    lcd.print("Do u want");
    lcd.setCursor(0, 1);
    lcd.print("thermometer?");
    lcd.setCursor(0, 2);
    lcd.print("Press Yes or No");
    lcd.setCursor(0, 3);
    lcd.print("Press Misc to end");
    lcd.setCursor(0, 0);
    while (digitalRead(yes) == HIGH && digitalRead(no) == HIGH && digitalRead(misc) == HIGH) {
    }
    if(digitalRead(yes) == LOW){
      wantTEMP=true;
      Serial.println("Yes");
      break;
    }
    else if (digitalRead(no) == LOW){
      Serial.println("No");
    }
    else{
      while (digitalRead(misc) == LOW) {
      }
      lcd.clear();
      lcd.setCursor(0, 1);
      lcd.print("Goodbye!");
      lcd.setCursor(0, 0);
      delay(2000);
      turnoff();
      break;
    }
    while (digitalRead(yes) == LOW || digitalRead(no) == LOW) {
    }
    lcd.clear();
    lcd.setCursor(0, 1);
    lcd.print("Goodbye!");
    lcd.setCursor(0, 0);
    delay(2000);
    turnoff();
    notBreak=false;
  }
}
void turnoff(){
  lcd.print("Turn LCD off");
  off=true;
  lcd.noBacklight();
}
void HRS(){
  rateSpot = 0;
  lastBeat = 0;
  byte rates[6];
  int iter = 0;
  float bpm = 0;
  lcd.print("Place finger firmly on");
  lcd.setCursor(0, 1);
  lcd.print("Heart rate sensor");
  lcd.setCursor(0, 0);
  lcd.clear();
  for (int x = 0; x < 10; x++){
    lcd.print("Calibrating.");
    delay(1000);
    lcd.clear();
    lcd.print("Calibrating..");
    delay(1000);
    lcd.clear();
    lcd.print("Calibrating...");
    delay(1000);
    lcd.clear();
  }
  while(iter<1000){ //Change to how many times we want to loop through the sensing, then print out final average
    irValue = GLC.getIR();

  if (checkForBeat(irValue) == true)
  {
    //We sensed a beat!
    long delta = millis() - lastBeat;
    lastBeat = millis();

    beatsPerMinute = 60 / (delta / 1000.0);

    if (beatsPerMinute < 255 && beatsPerMinute > 20)
    {
      rates[rateSpot++] = (byte)beatsPerMinute; //Store this reading in the array
      rateSpot %= 6; //Wrap variable

      //Take average of readings
      beatAvg = 0;
      for (byte x = 0 ; x < 6 ; x++)
        beatAvg += rates[x];
      beatAvg /= 6;
    }
    bpm+=beatsPerMinute;
  }
  else{
    lcd.print("Please try again");
    return;
  }
  lcd.print("BPM is: ");
  lcd.print(beatsPerMinute);
  lcd.clear();
    iter++;
  }
  lcd.print("Avg is: ");
  lcd.print(bpm/iter);
  delay(10000);
  wantHRS=false;
}
void temperature(){
  int iter=0;
  int tempF=0;
  while(iter<100){
  tempF+=0;
  iter++;
  }
  tempF =tempF/iter;
  lcd.print(tempF);
  delay(2000);
  wantTEMP=false;
}
void printFrame(int i) {
    lcd.clear();
    char line[21]; // 20 chars + null

    // Top border
    strcpy(line, "+------------------+");
    lcd.setCursor(0, 0);
    lcd.print(line);

    // Eyes row — dynamically center
    memset(line, ' ', 20);
    line[0] = '|';
    line[19] = '|';
    int startCol = (18 - strlen(eyes[i])) / 2 + 1; // 18 = inner width, +1 for left border
    memcpy(line + startCol, eyes[i], strlen(eyes[i]));
    line[20] = '\0';
    lcd.setCursor(0, 1);
    lcd.print(line);

    // Mouth row — dynamically center
    memset(line, ' ', 20);
    line[0] = '|';
    line[19] = '|';
    int startColMouth = (18 - strlen(mouths[i])) / 2 + 1;
    memcpy(line + startColMouth, mouths[i], strlen(mouths[i]));
    line[20] = '\0';
    lcd.setCursor(0, 2);
    lcd.print(line);

    // Bottom border
    strcpy(line, "+------------------+");
    lcd.setCursor(0, 3);
    lcd.print(line);
}
void turnon(){
  lcd.backlight();

  const int frameCount = sizeof(eyes) / sizeof(eyes[0]);

  for (int i = 0; i < frameCount; i++) {
        printFrame(i);
        delay(50);
    }
  delay(200);
  lcd.clear();
  lcd.print("Welcome to:");
  lcd.setCursor(0, 1);
  lcd.print("Beep Bop Easy Doc");
  delay(2000);
  notBreak=true;
  off=false;
}
void loop() {
  if(wantHRS){
    HRS();
  }
  if(wantTEMP){
    temperature();
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
  if(!wantHRS&&!wantTEMP){
    prompt();
  }
  delay(200);
  lcd.clear();
}



