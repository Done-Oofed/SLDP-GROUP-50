#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <MAX30105.h> //GLC library
#include "heartRate.h"
#include "spo2_algorithm.h"
#include <avr/io.h>
#include <avr/pgmspace.h>
uint8_t flags = 0;
#define WANT_HRS   0x01
#define WANT_TEMP  0x02
#define WANT_SPO2  0x04
#define IS_OFF     0x08
#define NOT_BREAK  0x10
#define WANT_GLUC  0x20
const char eyes0[] PROGMEM = "=== ===";
const char eyes1[] PROGMEM = "--- ---";
const char eyes2[] PROGMEM = "-   -";
const char eyes3[] PROGMEM = "o   o";
const char eyes4[] PROGMEM = "O   O";
const char eyes5[] PROGMEM = "-   -";
const char eyes6[] PROGMEM = "--- ---";
const char eyes7[] PROGMEM = "-   -";
const char eyes8[] PROGMEM = "o   o";
const char eyes9[] PROGMEM = "O   O";
const char mouths0[] PROGMEM = "___";
const char mouths1[] PROGMEM = "---";
const char mouths2[] PROGMEM = "_ _";
const char mouths3[] PROGMEM = ". .";
const char mouths4[] PROGMEM = "_._";
const char mouths5[] PROGMEM = "...";
const char mouths6[] PROGMEM = "_..";
const char mouths7[] PROGMEM = "-.-";
const char mouths8[] PROGMEM = "._.";
const char mouths9[] PROGMEM = "---";
const char* const eyes[] PROGMEM = {eyes0, eyes1, eyes2, eyes3, eyes4, eyes5, eyes6, eyes7, eyes8, eyes9};
const char* const mouths[] PROGMEM = {mouths0, mouths1, mouths2, mouths3, mouths4, mouths5, mouths6, mouths7, mouths8, mouths9};

byte sp = 0;
byte beats = 0;

LiquidCrystal_I2C lcd(0x27,20,4);
MAX30105 GLC;
void calibrate() {
  for (byte x = 0; x < 3; x++) {
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
}
void showProgress(byte iter, int total) {
  lcd.print(F("      Progress"));
  lcd.setCursor(0, 1);
  lcd.print(F("["));
  for (byte i = 0; i < 18; i++) {
    if (i < (iter * 18) / total) lcd.print(F("#"));
    else lcd.print(F("-"));
  }
  lcd.print(F("]"));
  lcd.setCursor(0, 2);
  lcd.print(F("    Keep Holding"));
  lcd.setCursor(0, 0);
}
void displayResult(float value, const char* label, const char* unit, float high, float low, const char* highMsg, const char* lowMsg) {
  lcd.clear();
  lcd.print(label);
  lcd.print(value);
  lcd.print(unit);
  delay(10000);
  lcd.clear();
  if (value > high) {
    lcd.print(highMsg);
    lcd.setCursor(0, 1);
    lcd.print(F("too high, please"));
    lcd.setCursor(0, 2);
    lcd.print(F("see a doctor"));
    lcd.setCursor(0, 0);
    delay(10000);
    lcd.clear();
  } else if (value < low) {
    lcd.print(lowMsg);
    lcd.setCursor(0, 1);
    lcd.print(F("too low, please"));
    lcd.setCursor(0, 2);
    lcd.print(F("see a doctor"));
    lcd.setCursor(0, 0);
    delay(10000);
    lcd.clear();
  }
}
bool askYesNo(const char* question) {
  lcd.clear();
  lcd.print(F("Do u want"));
  lcd.setCursor(0, 1);
  lcd.print(question);
  lcd.setCursor(0, 2);
  lcd.print(F("Press Yes or No"));
  lcd.setCursor(0, 3);
  lcd.print(F("Press Misc to end"));
  lcd.setCursor(0, 0);
  while (digitalRead(2) == HIGH && digitalRead(3) == HIGH && digitalRead(5) == HIGH) {}
  if (digitalRead(2) == LOW) {
    Serial.println(F("Yes"));
    while (digitalRead(2) == LOW) {}
    return true;
  } else if (digitalRead(3) == LOW) {
    Serial.println(F("No"));
    while (digitalRead(3) == LOW) {}
    return false;
  } else {
    while (digitalRead(5) == LOW) {}
    lcd.clear();
    lcd.setCursor(0, 1);
    lcd.print(F("Goodbye!"));
    lcd.setCursor(0, 0);
    delay(2000);
    turnoff();
    return false;
  }
}
void setup() {
  Serial.begin(115200);
  lcd.init();
  if (!GLC.begin(Wire, I2C_SPEED_FAST)) {
    Serial.println(F("MAX30102 was not found. Please check wiring/power."));
  }
  GLC.setup();
  GLC.setPulseAmplitudeRed(0x0A);
  GLC.enableDIETEMPRDY();
  GLC.setPulseAmplitudeGreen(0);
  flags |= NOT_BREAK; 
  flags |= IS_OFF;
}
void prompt(){
    while (flags & NOT_BREAK) {
    if (askYesNo("heart beat?")) { flags |= WANT_HRS; break; }
    if (askYesNo("SPO2?")) { flags |= WANT_SPO2; break; }
    if (askYesNo("Glucose?")) { flags |= WANT_GLUC; break; }
    if (askYesNo("thermometer?")) { flags |= WANT_TEMP; break; }
    lcd.clear();
    lcd.setCursor(0, 1);
    lcd.print(F("Goodbye!"));
    lcd.setCursor(0, 0);
    delay(2000);
    turnoff();
    flags &= ~NOT_BREAK;
  }
}
void turnoff(){
  lcd.print(F("Turn LCD off"));
  flags |= IS_OFF;
  lcd.noBacklight();
}
void HRS(bool want){
  byte rates[6] = {0};
  byte rateSpot = 0;
  unsigned long lastBeat = 0;
  float beatsPerMinute = 0;
  byte iter = 0;
  float bpm = 0;
  byte check = 0;
  if(!want){
  lcd.print(F("Place finger firmly"));
  lcd.setCursor(0, 1);
  lcd.print(F("on sensor"));
  lcd.setCursor(0, 0);
  delay(5000);
  lcd.clear();
  calibrate();
  }
  while (iter < 250) {
    int qm = GLC.getIR();
    if (checkForBeat(qm) == true) {
      long delta = millis() - lastBeat;
      lastBeat = millis();
      beatsPerMinute = 60 / (delta / 1000.0);
      if (beatsPerMinute < 255 && beatsPerMinute > 20) {
        bpm += beatsPerMinute;
        check++;
        rates[rateSpot++] = (byte)beatsPerMinute;
        rateSpot %= 6;
      }
    }
    if (iter % 25 == 0) showProgress(iter, 250);
    iter++;
  }
  if(want){
    beats = bpm/check;
  }
  else{
    Serial.println(bpm);
    Serial.println(check);
  displayResult(bpm / check, "Avg is: ", "", 100, 60, "Your heartrate is", "Your heartrate is");
  flags &= ~WANT_HRS;
  }
}
void temperature(){
  byte iter = 0;
  float tempF = 0;
  lcd.print(F("Place finger firmly"));
  lcd.setCursor(0, 1);
  lcd.print(F("on sensor"));
  lcd.setCursor(0, 0);
  delay(5000);
  lcd.clear();
  calibrate();
  while (iter < 250) {
    tempF = GLC.readTemperatureF();
    if (iter % 25 == 0) showProgress(iter, 250);
    iter++;
  }
  displayResult(tempF, "Temperature: ", "F", 100, 60, "Your temperature is", "Your temperature is");
  flags &= ~WANT_TEMP;
}
void SP02(bool want){
  int32_t bufferLength = 50;
  uint16_t irBuffer[50];
  uint16_t redBuffer[50];
  int32_t spo2 = 0;
  int8_t validSPO2 = 0;
  int32_t heartRate = 0;
  int8_t validHeartRate = 0;
  byte max = 0;
  lcd.print(F("Place finger firmly"));
  lcd.setCursor(0, 1);
  lcd.print(F("on sensor"));
  lcd.setCursor(0, 0);
  delay(5000);
  lcd.clear();
  calibrate();
  for (byte i = 0; i < bufferLength; i++) {
    while (GLC.available() == false) GLC.check();
    redBuffer[i] = GLC.getRed();
    irBuffer[i] = GLC.getIR();
    GLC.nextSample();
  }
  maxim_heart_rate_and_oxygen_saturation(irBuffer, bufferLength, redBuffer, &spo2, &validSPO2, &heartRate, &validHeartRate);
 for(byte a = 0; a<20; a++)
  {
    //dumping the first 25 sets of samples in the memory and shift the last 75 sets of samples to the top
    for (byte i = 25; i < 50; i++)
    {
      redBuffer[i - 25] = redBuffer[i];
      irBuffer[i - 25] = irBuffer[i];
    }

    //take 25 sets of samples before calculating the heart rate.
    for (byte i = 25; i < 50; i++)
    {
      while (GLC.available() == false) //do we have new data?
        GLC.check(); //Check the sensor for new data

      redBuffer[i] = GLC.getRed();
      irBuffer[i] = GLC.getIR();
      GLC.nextSample();

      Serial.print(F("red="));
      Serial.print(redBuffer[i], DEC);
      Serial.print(F(", ir="));
      Serial.print(irBuffer[i], DEC);

      Serial.print(F(", HR="));
      Serial.print(heartRate, DEC);

      Serial.print(F(", HRvalid="));
      Serial.print(validHeartRate, DEC);

      Serial.print(F(", SPO2="));
      Serial.print(spo2, DEC);

      Serial.print(F(", SPO2Valid="));
      Serial.println(validSPO2, DEC);
    }
    maxim_heart_rate_and_oxygen_saturation(irBuffer, bufferLength, redBuffer, &spo2, &validSPO2, &heartRate, &validHeartRate);
    //After gathering 25 new samples recalculate HR and SP02
    if(max < spo2){
      max = spo2;
    }
  }
  if(want){
    sp = max;
  }
  else{
  displayResult(max, "SP02: ", "", 120, 60, "Your SPO2 is", "Your SPO2 is");
  flags &= ~WANT_SPO2;
  }
}
void printFrame(int i) {
  char eyeStr[10];
  char mouthStr[5];
  strcpy_P(eyeStr, (char*)pgm_read_word(&eyes[i]));
  strcpy_P(mouthStr, (char*)pgm_read_word(&mouths[i]));
  lcd.clear();
  char line[21];
  strcpy(line, "+------------------+");
  lcd.setCursor(0, 0);
  lcd.print(line);
  memset(line, ' ', 20);
  line[0] = '|';
  line[19] = '|';
  int startCol = (18 - strlen(eyeStr)) / 2 + 1;
  memcpy(line + startCol, eyeStr, strlen(eyeStr));
  line[20] = '\0';
  lcd.setCursor(0, 1);
  lcd.print(line);
  memset(line, ' ', 20);
  line[0] = '|';
  line[19] = '|';
  int startColMouth = (18 - strlen(mouthStr)) / 2 + 1;
  memcpy(line + startColMouth, mouthStr, strlen(mouthStr));
  line[20] = '\0';
  lcd.setCursor(0, 2);
  lcd.print(line);
  strcpy(line, "+------------------+");
  lcd.setCursor(0, 3);
  lcd.print(line);
}
void GLUC(){
  SP02(true);
  HRS(true);
  byte eq = 16714.61 + .47*beats - 351.045*sp + 1.85 * (sp*sp);
  byte but = 0;
  lcd.clear();
  lcd.print(F("Did you recently"));
  lcd.setCursor(0,1);
  lcd.print(F("eat food?"));
  while (digitalRead(2) == HIGH && digitalRead(3) == HIGH) {} //2 yes 3 no
  if(digitalRead(2)==HIGH){
    but = 2;
  }
  else{
    but = 3;
  }
  while (digitalRead(2) == LOW || digitalRead(3) == LOW) {}
  if(but == 2){
    displayResult(eq, "Glucose: ", "", 120, 60, "Your Glucose is", "Your Glucose is");
  }
  else{
    displayResult(eq, "Glucose: ", "", 100, 60, "Your Glucose is", "Your Glucose is");
  }
  flags &= ~WANT_GLUC;
}
void turnon() {
  lcd.backlight();
  const int frameCount = 10;
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
  flags |= NOT_BREAK;
  flags &= ~IS_OFF;
}
void loop() {
  if (flags & WANT_HRS) HRS(false);
  if (flags & WANT_TEMP) temperature();
  if (flags & WANT_SPO2) SP02(false);
  if (flags & WANT_GLUC) GLUC();
  while (flags & IS_OFF) {
    Serial.println(F("LCD off"));
    while (digitalRead(5) == HIGH) {}
    Serial.println(F("Button pressed"));
    turnon();
    while (digitalRead(5) == LOW) {}
  }
  if (!(flags & (WANT_HRS | WANT_TEMP | WANT_SPO2 | WANT_GLUC))) prompt();
  delay(200);
  lcd.clear();
}



