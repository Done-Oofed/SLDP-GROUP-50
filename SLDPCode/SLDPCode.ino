#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <MAX30105.h> //GLC library
//#include <string.h>
#include "spo2_algorithm.h"
#include <avr/io.h>
bool wantHRS = false;
bool wantTEMP = false;
bool wantSPO2 = false;
bool off = true;
bool notBreak = true;
String sys
LiquidCrystal_I2C lcd(0x27,20,4);
MAX30105 GLC;

const char eyes_0[] PROGMEM = "=== ===";
const char eyes_1[] PROGMEM = "--- ---";
const char eyes_2[] PROGMEM = "-   -";
const char eyes_3[] PROGMEM = "o   o";
const char eyes_4[] PROGMEM = "O   O";
const char eyes_5[] PROGMEM = "-   -";
const char eyes_6[] PROGMEM = "--- ---";
const char eyes_7[] PROGMEM = "-   -";
const char eyes_8[] PROGMEM = "o   o";
const char eyes_9[] PROGMEM = "O   O";

const char* const eyes[] PROGMEM = {
  eyes_0, eyes_1, eyes_2, eyes_3, eyes_4,
  eyes_5, eyes_6, eyes_7, eyes_8, eyes_9
};

// init  faces for screen to PROG MEM (save space from dRAM)
const char mouths_0[] PROGMEM = "___";
const char mouths_1[] PROGMEM = "---";
const char mouths_2[] PROGMEM = "_ _";
const char mouths_3[] PROGMEM = ". .";
const char mouths_4[] PROGMEM = "_._";
const char mouths_5[] PROGMEM = "...";
const char mouths_6[] PROGMEM = "_..";
const char mouths_7[] PROGMEM = "-.-";
const char mouths_8[] PROGMEM = "._.";
const char mouths_9[] PROGMEM = "---";

const char* const mouths[] PROGMEM = {
  mouths_0, mouths_1, mouths_2, mouths_3, mouths_4,
  mouths_5, mouths_6, mouths_7, mouths_8, mouths_9
};
const uint8_t frameCount = sizeof(eyes) / sizeof(eyes[0]);

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

void turnoff(){
  lcd.print(F("Turn LCD off"));
  off=true;
  lcd.noBacklight();
}
//Dont worry about prompt(), it should work correctly and only changes the wantHBS and wantTemp if user presses yes
void questioning(bool sensor){
  String method;

  if (sensor == "wantHRS"){
    method = "Heart Rate";
  }
  else if (sensor == "wantTEMP"){
    method = "Temperature";
  }
  else if (sensor == "wantSPO2"){
    method = "Oxygen Levels";
  }

	while(sensor == false){
		while (digitalRead(2) == LOW || digitalRead(3) == LOW) {
    }
    lcd.clear();
		lcd.print(F("Do u want"));
    lcd.setCursor(0,1);
    lcd.print(method);
    lcd.setCursor(0,2);
    lcd.print(F("Press Yes or No"));
    lcd.setCursor(0,3);
    lcd.print(F("Press Misc to end"));
    lcd.setCursor(0,0);
    
    while (digitalRead(2) == HIGH && digitalRead(3) == HIGH && digitalRead(5) == HIGH){
    }
    if (digitalRead(2) == LOW){
      Serial.println(F("Yes"));
      sensor = true;
      break;
    } else if (digitalRead(3) == LOW){
      Serial.println(F("No"));
    }
    else{
      while (digitalRead(5) == LOW){
      }
      lcd.clear();
      lcd.setCursor(0,1);
      lcd.print(F("Goodbye!"));
      lcd.setCursor(0,0);
      delay(2000);
      turnoff();
      break;
    }
  }
}

void prompt(){
  questioning(wantHRS);
  questioning(wantTEMP);
  questioning(wantSPO2);
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

void drawCenteredRow(uint8_t row, const char* str) {
  char buffer[21];
  memset(buffer, ' ', 20);
  buffer[0] = '|';
  buffer[19] = '|';

  char temp[20];
  strcpy_P(temp, str);

  int offset = (18 - strlen(temp)) / 2 + 1;
  memcpy(buffer + offset, temp, strlen(temp));

  buffer[20] = '\0';
  lcd.setCursor(0, row);
  lcd.print(buffer);
}

void printFrame(uint8_t i) {
  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print(F("+------------------+"));

  drawCenteredRow(1, (PGM_P)pgm_read_ptr(&eyes[i]));
  drawCenteredRow(2, (PGM_P)pgm_read_ptr(&mouths[i]));

  lcd.setCursor(0, 3);
  lcd.print(F("+------------------+"));
}

void turnon() {
  lcd.backlight();

  for (uint8_t i = 0; i < frameCount; i++) {
    printFrame(i);
    delay(50);
  }

  delay(200);
  lcd.clear();
  lcd.print(F("Welcome to:"));
  lcd.setCursor(0, 1);
  lcd.print(F("Beep Bop Easy Doc"));

  delay(2000);
  notBreak = true;
  off = false;
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
    Serial.println(freeMemory());
    //turnon();
    Serial.println(freeMemory());
    while (digitalRead(5) == LOW) {
    }
  }
  if(!wantHRS&&!wantTEMP&&!wantSPO2){
    prompt();
  }
  delay(200);
  lcd.clear();
}



