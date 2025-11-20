#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <MAX30105.h> //GLC library
#include "heartRate.h"
#include "spo2_algorithm.h"
#include <avr/io.h>
bool wantHRS = false;
bool wantTEMP = false;
bool wantSPO2 = false;
bool off = true;
bool notBreak = true;

LiquidCrystal_I2C lcd(0x27,20,4);
MAX30105 GLC;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  lcd.init();
  if (!GLC.begin(Wire, I2C_SPEED_FAST)) { // Start communication using fast I2C speed
    Serial.println(F("MAX30102 was not found. Please check wiring/power. "));
  }
  GLC.setup(); // Configure sensor with default settings for heart rate monitoring
  GLC.setPulseAmplitudeRed(0x0A); // Set the red LED pulse amplitude (intensity) to a low value as an indicator
  GLC.enableDIETEMPRDY();
  GLC.setPulseAmplitudeGreen(0);
}
//Dont worry about prompt(), it should work correctly and only changes the wantHBS and wantTemp if user presses yes
void prompt(){
    while(notBreak){
    lcd.clear();
    lcd.print(F("Do u want"));
    lcd.setCursor(0, 1);
    lcd.print(F("heart beat?"));
    lcd.setCursor(0, 2);
    lcd.print(F("Press Yes or No"));
    lcd.setCursor(0, 3);
    lcd.print(F("Press Misc to end"));
    lcd.setCursor(0, 0);
    while (digitalRead(2) == HIGH && digitalRead(3) == HIGH && digitalRead(5) == HIGH) {
    }
    if(digitalRead(2) == LOW){
      Serial.println(F("Yes"));
      wantHRS = true;
      break;
    }
    else if (digitalRead(3) == LOW){
      Serial.println(F("No"));
    }
    else{
      while (digitalRead(5) == LOW) {
      }
      lcd.clear();
      lcd.setCursor(0, 1);
      lcd.print(F("Goodbye!"));
      lcd.setCursor(0, 0);
      delay(2000);
      turnoff();
      break;
    }
    while (digitalRead(2) == LOW || digitalRead(3) == LOW) {
    }
    lcd.clear();
    lcd.print(F("Do u want"));
    lcd.setCursor(0, 1);
    lcd.print(F("SPO2?"));
    lcd.setCursor(0, 2);
    lcd.print(F("Press Yes or No"));
    lcd.setCursor(0, 3);
    lcd.print(F("Press Misc to end"));
    lcd.setCursor(0, 0);
    while (digitalRead(2) == HIGH && digitalRead(3) == HIGH && digitalRead(5) == HIGH) {
    }
    if(digitalRead(2) == LOW){
      Serial.println(F("Yes"));
      wantSPO2 = true;
      break;
    }
    else if (digitalRead(3) == LOW){
      Serial.println(F("No"));
    }
    else{
      while (digitalRead(5) == LOW) {
      }
      lcd.clear();
      lcd.setCursor(0, 1);
      lcd.print(F("Goodbye!"));
      lcd.setCursor(0, 0);
      delay(2000);
      turnoff();
      break;
    }
    while (digitalRead(2) == LOW || digitalRead(3) == LOW) {
    }
    lcd.clear();
    lcd.print(F("Do u want"));
    lcd.setCursor(0, 1);
    lcd.print(F("blood pressure?"));
    lcd.setCursor(0, 2);
    lcd.print(F("Press Yes or No"));
    lcd.setCursor(0, 3);
    lcd.print(F("Press Misc to end"));
    lcd.setCursor(0, 0);
    while (digitalRead(2) == HIGH && digitalRead(3) == HIGH && digitalRead(5) == HIGH) {
    }
    if(digitalRead(2) == LOW){
      Serial.println(F("Yes"));
      break;
    }
    else if (digitalRead(3) == LOW){
      Serial.println(F("No"));
    }
    else{
      while (digitalRead(5) == LOW) {
      }
      lcd.clear();
      lcd.setCursor(0, 1);
      lcd.print(F("Goodbye!"));
      lcd.setCursor(0, 0);
      delay(2000);
      turnoff();
      break;
    }
    while (digitalRead(2) == LOW || digitalRead(3) == LOW) {
    }
    lcd.clear();
    lcd.print(F("Do u want"));
    lcd.setCursor(0, 1);
    lcd.print(F("thermometer?"));
    lcd.setCursor(0, 2);
    lcd.print(F("Press Yes or No"));
    lcd.setCursor(0, 3);
    lcd.print(F("Press Misc to end"));
    lcd.setCursor(0, 0);
    while (digitalRead(2) == HIGH && digitalRead(3) == HIGH && digitalRead(5) == HIGH) {
    }
    if(digitalRead(2) == LOW){
      wantTEMP=true;
      Serial.println(F("Yes"));
      break;
    }
    else if (digitalRead(3) == LOW){
      Serial.println(F("No"));
    }
    else{
      while (digitalRead(5) == LOW) {
      }
      lcd.clear();
      lcd.setCursor(0, 1);
      lcd.print(F("Goodbye!"));
      lcd.setCursor(0, 0);
      delay(2000);
      turnoff();
      break;
    }
    while (digitalRead(2) == LOW || digitalRead(3) == LOW) {
    }
    lcd.clear();
    lcd.setCursor(0, 1);
    lcd.print(F("Goodbye!"));
    lcd.setCursor(0, 0);
    delay(2000);
    turnoff();
    notBreak=false;
  }
}
void turnoff(){
  lcd.print(F("Turn LCD off"));
  off=true;
  lcd.noBacklight();
}
void HRS(){
    long irValue = 0;
  byte rates[6] = {0};
  byte rateSpot = 0;
  unsigned long lastBeat = 0;
  float beatsPerMinute = 0;
  int16_t beatAvg = 0;

  int iter = 0;
  float bpm = 0;
  int check = 0;
  lcd.print(F("Place finger firmly"));
  lcd.setCursor(0, 1);
  lcd.print(F("on heartrate sensor"));
  lcd.setCursor(0, 0);
  delay(5000);
  lcd.clear();
  for (int x = 0; x < 3; x++){
    lcd.print(F("Calibrating."));
    delay(1000);
    lcd.clear();
    lcd.print(F("Calibrating.."));
    delay(1000);
    lcd.clear();
    lcd.print(F("Calibrating..."));
    delay(1000);
    lcd.clear();
  }
  while(iter<500){ //Change to how many times we want to loop through the sensing, then print out final average
    irValue = GLC.getIR();
  if (checkForBeat(irValue) == true)
  {
    //We sensed a beat!
    long delta = millis() - lastBeat;
    lastBeat = millis();

    beatsPerMinute = 60 / (delta / 1000.0);

    if (beatsPerMinute < 255 && beatsPerMinute > 20)
    {
      bpm += beatsPerMinute;
      check++;
      rates[rateSpot++] = (byte)beatsPerMinute; //Store this reading in the array
      rateSpot %= 6; //Wrap variable

      //Take average of readings
      beatAvg = 0;
      for (byte x = 0 ; x < 6 ; x++)
        beatAvg += rates[x];
      beatAvg /= 6;
    }
  }
  if(iter % 5 == 0){
    lcd.print(F("      Progress"));
    lcd.setCursor(0, 1);
    lcd.print(F("["));
    for (int i = 0; i < 18; i++) {
        if (i < (iter * 18) / 500) lcd.print(F("#")); //Change w total
        else lcd.print(F("-"));
    }
    lcd.print(F("]"));
    lcd.setCursor(0, 2);
    lcd.print(F("    Keep Holding"));
    lcd.setCursor(0, 0);
  }
    iter++;
  }
  lcd.clear();
  lcd.print(F("Avg is: "));
  lcd.print(bpm/check);
  delay(10000);
  lcd.clear();
  if(bpm/check>100){
    lcd.print(F("Your heartrate is"));
    lcd.setCursor(0, 1);
    lcd.print(F("too high, please"));
    lcd.setCursor(0, 2);
    lcd.print(F("see a doctor"));
    lcd.setCursor(0, 0);
    delay(10000);
    lcd.clear();
  }
  else if(bpm/check<60){
    lcd.print(F("Your heartrate is"));
    lcd.setCursor(0, 1);
    lcd.print(F("too low, please"));
    lcd.setCursor(0, 2);
    lcd.print(F("see a doctor"));
    lcd.setCursor(0, 0);
    delay(10000);
    lcd.clear();
  }
  wantHRS=false;
}
void temperature(){
  int iter = 0;
  float tempF = 0;
  int check = 0;
  lcd.print(F("Place finger firmly"));
  lcd.setCursor(0, 1);
  lcd.print(F("on temp sensor"));
  lcd.setCursor(0, 0);
  delay(5000);
  lcd.clear();
  for (int x = 0; x < 3; x++){
    lcd.print(F("Calibrating."));
    delay(1000);
    lcd.clear();
    lcd.print(F("Calibrating.."));
    delay(1000);
    lcd.clear();
    lcd.print(F("Calibrating..."));
    delay(1000);
    lcd.clear();
  }
  while(iter<500){
    tempF = GLC.readTemperatureF();
  if(iter % 5 == 0){
  lcd.print(F("      Progress"));
    lcd.setCursor(0, 1);
    lcd.print(F("["));
    for (int i = 0; i < 18; i++) {
        if (i < (iter * 18) / 500) lcd.print(F("#")); //Change w total
        else lcd.print(F("-"));
    }
    lcd.print(F("]"));
    lcd.setCursor(0, 2);
    lcd.print(F("    Keep Holding"));
    lcd.setCursor(0, 0);
    Serial.print(iter);
  }
  iter++;
  }
  lcd.clear();
  lcd.print(F("Temperature: "));
  lcd.print(tempF);
  lcd.print(F("F"));
  delay(10000);
  lcd.clear();
  if(tempF>100){
    lcd.print(F("Your temperature is"));
    lcd.setCursor(0, 1);
    lcd.print(F("too high, please"));
    lcd.setCursor(0, 2);
    lcd.print(F("cool down"));
    lcd.setCursor(0, 0);
    delay(10000);
    lcd.clear();
  }
  else if(tempF<60){
    lcd.print(F("Your temperature is"));
    lcd.setCursor(0, 1);
    lcd.print(F("too low, please"));
    lcd.setCursor(0, 2);
    lcd.print(F("heat up"));
    lcd.setCursor(0, 0);
    delay(10000);
    lcd.clear();
  }
  wantTEMP=false;
}
int freeMemory() {
       extern int __heap_start, *__brkval;
       int v;
       int free = (int)&v - (__brkval == 0 ? (int)&__heap_start : (int)__brkval);
       return free > 0 ? free : 0;  // Prevent negative values
     }
void SP02(){
  // moved here to save persistent global RAM (used only during SP02 measurements)
  int32_t bufferLength = 50; // data length (samples)
  uint16_t irBuffer[50]; // infrared LED sensor data (temporary)
  uint16_t redBuffer[50]; // red LED sensor data (temporary)
  int32_t spo2 = 0; // SPO2 value (temporary)
  int8_t validSPO2 = 0; // indicator (temporary)
  int32_t heartRate = 0; // heart rate value (temporary)
  int8_t validHeartRate = 0; // indicator (temporary)
  int iter = 0;
  int check = 0;
  lcd.print(F("Place finger firmly"));
  lcd.setCursor(0, 1);
  lcd.print(F("on SP02 sensor"));
  lcd.setCursor(0, 0);
  delay(5000);
  lcd.clear();
  for (int x = 0; x < 3; x++){
    lcd.print(F("Calibrating."));
    delay(1000);
    lcd.clear();
    lcd.print(F("Calibrating.."));
    delay(1000);
    lcd.clear();
    lcd.print(F("Calibrating..."));
    delay(1000);
    lcd.clear();
  }
  for (byte i = 0 ; i < 50; i++)
  {
    while (GLC.available() == false) //do we have new data?
      GLC.check();

    redBuffer[i] = GLC.getRed();
    irBuffer[i] = GLC.getIR();
    GLC.nextSample(); //We're finished with this sample so move to next sample

    Serial.print(F("red="));
    Serial.print(redBuffer[i], DEC);
    Serial.print(F(", ir="));
    Serial.println(irBuffer[i], DEC);
  }
  maxim_heart_rate_and_oxygen_saturation(irBuffer, bufferLength, redBuffer, &spo2, &validSPO2, &heartRate, &validHeartRate);
  
  
  lcd.clear();
  lcd.print(F("SP02: "));
  lcd.print(spo2);
  delay(10000);
  lcd.clear();
  if(spo2>100){
    lcd.print(F("Your temperature is"));
    lcd.setCursor(0, 1);
    lcd.print(F("too high, please"));
    lcd.setCursor(0, 2);
    lcd.print(F("cool down"));
    lcd.setCursor(0, 0);
    delay(10000);
    lcd.clear();
  }
  else if(spo2<60){
    lcd.print(F("Your temperature is"));
    lcd.setCursor(0, 1);
    lcd.print(F("too low, please"));
    lcd.setCursor(0, 2);
    lcd.print(F("heat up"));
    lcd.setCursor(0, 0);
    delay(10000);
    lcd.clear();
  }
  wantSPO2=false;
}
void printFrame(int i) {
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
  lcd.backlight();

  const int frameCount = sizeof(eyes) / sizeof(eyes[0]);

  for (int i = 0; i < frameCount; i++) {
        printFrame(i);
        delay(50);
    }
  delay(200);
  lcd.clear();
  lcd.print(F("Welcome to:"));
  lcd.setCursor(0, 1);
  lcd.print(F("Beep Bop Easy Doc"));
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
  if(wantSPO2){
    SP02();
  }
  while(off){
    Serial.println(F("LCD off"));
    while (digitalRead(5) == HIGH) {
    }
    Serial.println(F("Button pressed"));
    turnon();
    while (digitalRead(5) == LOW) {
    }
  }
  if(!wantHRS&&!wantTEMP&&!wantSPO2){
    prompt();
  }
  delay(200);
  lcd.clear();
}



