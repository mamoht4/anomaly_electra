#include "Arduino.h"
#include "SoftwareSerial.h"
#include "DFRobotDFPlayerMini.h"
SoftwareSerial mySoftwareSerial(4, 3); // RX, TX
int playerBusyPin = 2; //LOW - busy/played audio, HIGH - ready
bool playerPlayed = 0;
DFRobotDFPlayerMini myDFPlayer;
void printDetail(uint8_t type, int value);

int ledPins[] = {5, 6, 7, 8};
int NUM_LEDS;
bool d1; 
int autoModePin = 19; // HIGH - постоянно, LOW - по датчикам
int builtinLedPin = 13;
int photoResistPin = 14;
int dopplerRadarPin = 15;
unsigned long timerU;

#define TIMEU 10000UL // 4 сек для теста. рабочее = 10 ??? сек работы после сработки (мб поправить!!!)

void setup() {
  mySoftwareSerial.begin(9600);
  Serial.begin(9600);
  delay(10000);
  if (!myDFPlayer.begin(mySoftwareSerial)) {  //Use softwareSerial to communicate with mp3.
    Serial.println(F("DFPlayer Mini not ready"));
    while(true){
      delay(5); // Code to compatible with ESP8266 watch dog.
    }
  }
  Serial.println("DFPlayer Mini online");
  pinMode(playerBusyPin, INPUT);
  
//  myDFPlayer.volume(30);  //Set volume value. From 0 to 30
//  myDFPlayer.play(1);  //Play the first mp3
//  delay(500);
//  myDFPlayer.play(2);
//  delay(500);
//  myDFPlayer.play(3);
  
  NUM_LEDS = sizeof(ledPins) / sizeof(int);
  pinMode(autoModePin, INPUT);
  randomSeed(analogRead(A0));
  for (int n = 0; n < NUM_LEDS; n++) {
    pinMode(ledPins[n], OUTPUT);
  }
  pinMode(builtinLedPin, OUTPUT);
}

void loop() {
  if (myDFPlayer.available()) {
    printDetail(myDFPlayer.readType(), myDFPlayer.read()); //Print the detail message from DFPlayer to handle different errors and states.
  }
//  Serial.println(analogRead(playerBusyPin));
//  Serial.println(digitalRead(playerBusyPin));

  if (digitalRead(autoModePin) == HIGH || digitalRead(photoResistPin) == LOW || digitalRead(dopplerRadarPin) == HIGH){
    d1 = 1;
    timerU = millis();
  }
  else {
    //если датчик обнаруживает, и ledPinы уже работают, но давнее TIMEU
    if ((digitalRead(photoResistPin) == LOW || digitalRead(dopplerRadarPin) == HIGH) && d1 == 1 && millis() - timerU > TIMEU) {
      timerU = millis();
    }
    //если датчик не обнаруживает, и уже сработало, но давнее TIMEU, и гром не играет
    if ((digitalRead(photoResistPin) == HIGH || digitalRead(dopplerRadarPin) == LOW) && d1 == 1 && millis() - timerU > TIMEU && playerPlayed == 0) {
      d1 = 0;
//      allOff();
      delay(500); //пауза для устранения быстрого повторного срабаывания
    }
  }
  
  if (d1 == 1) {
    if (playerPlayed == 0) {
      playerPlayed = 1;
//      myDFPlayer.play(1);
      myDFPlayer.next();
    }
//    else {
//     myDFPlayer.next();
//    }
    static unsigned long timer = 0;
    static unsigned long interval = random(20, 500); //частота мигания если датчик обнаруживает (мб ПОПРАВИТЬ!!!)
    digitalWrite(builtinLedPin, HIGH);
    if (millis() - timer >= interval) {
      timer = millis();
      someOff();
      int thisLed = random(0, NUM_LEDS);
      digitalWrite(ledPins[thisLed], HIGH);
    }
//    digitalWrite(ledPins[0], HIGH);
//    delay(1000);
//    digitalWrite(ledPins[0], LOW);
//    digitalWrite(ledPins[1], HIGH);
//    delay(1000);
//    digitalWrite(ledPins[1], LOW);
//    digitalWrite(ledPins[2], HIGH);
//    delay(1000);
//    digitalWrite(ledPins[2], LOW);
//    digitalWrite(ledPins[3], HIGH);
//    delay(1000  );
//    digitalWrite(ledPins[3], LOW);

  }
  
  if (d1 == 0) {
    static unsigned long timer = 0;
    static unsigned long interval = random(60000, 120000); // set (60000-120000) (ПОПРАВИТЬ!!!)
    digitalWrite(builtinLedPin, LOW);
    if (millis() - timer >= interval) {
      timer = millis();
      allOff();
      int thisLed = random(0, NUM_LEDS);
      digitalWrite(ledPins[thisLed], HIGH);
      delay(10);
      digitalWrite(ledPins[thisLed], LOW);
    }
  }
  
}

void allOff() {
  for (int n = 0; n < NUM_LEDS; n++) {
    digitalWrite(ledPins[n], LOW);
  }
}

void someOff() {
  bool rndFlag = random(NUM_LEDS);
  if (rndFlag == false) {
    allOff();
  }
  else {
  digitalWrite(ledPins[random(0, NUM_LEDS)], LOW);
  }
}

void printDetail(uint8_t type, int value){
  switch (type) {
    case TimeOut:
      Serial.println(F("Time Out!"));
      break;
    case WrongStack:
      Serial.println(F("Stack Wrong!"));
      break;
    case DFPlayerCardInserted:
      Serial.println(F("Card Inserted!"));
      break;
    case DFPlayerCardRemoved:
      Serial.println(F("Card Removed!"));
      break;
    case DFPlayerCardOnline:
      Serial.println(F("Card Online!"));
      break;
    case DFPlayerUSBInserted:
      Serial.println("USB Inserted!");
      break;
    case DFPlayerUSBRemoved:
      Serial.println("USB Removed!");
      break;
    case DFPlayerPlayFinished:
      playerPlayed = 0;
      Serial.print(F("Number:"));
      Serial.print(value);
      Serial.println(F(" Play Finished!"));
      break;
    case DFPlayerError:
      Serial.print(F("DFPlayerError:"));
      switch (value) {
        case Busy:
          Serial.println(F("Card not found"));
          break;
        case Sleeping:
          Serial.println(F("Sleeping"));
          break;
        case SerialWrongStack:
          Serial.println(F("Get Wrong Stack"));
          break;
        case CheckSumNotMatch:
          Serial.println(F("Check Sum Not Match"));
          break;
        case FileIndexOut:
          Serial.println(F("File Index Out of Bound"));
          break;
        case FileMismatch:
          Serial.println(F("Cannot Find File"));
          break;
        case Advertise:
          Serial.println(F("In Advertise"));
          break;
        default:
          break;
      }
      break;
    default:
      break;
  }
}
