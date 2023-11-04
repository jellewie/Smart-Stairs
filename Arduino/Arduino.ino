#include <FastLED.h>
#define AverageAmount 8
#define AnalogScaler pow(2,(12 - 8))
#include "avg.h"

struct Step {
  byte SectionLength;
  bool State;
  int StayOnFor;
  AVG Average;
};
Step Stair[] = {Step{25},                               //The steps and their LED section length
                Step{27},
                Step{27},
                Step{26},
                Step{27},
                Step{25},
                Step{24},
                Step{26},
                Step{27},
                Step{26},
                Step{28},
                Step{26},
                Step{25}
               };
const byte PotMinChange = 2;
const byte PotStick = 2;
const byte TriggerThreshold = 60;
const int LEDTimeOn = 400;
const int LEDTimeIdle = 200;
CRGB LEDColorOn = {5, 2, 2};
CRGB LEDColorNextOn = {2, 1, 1};
CRGB LEDColorOff = {0, 0, 0};
CRGB LEDColorIdle = {1, 0, 0};
const byte PAO_LED = 5;
const int TotalLEDs = 13 * 30;
const byte PDO_S0 = 13;                                 //Connections to CD74HC4067
const byte PDO_S1 = 27;                                 //^
const byte PDO_S2 = 33;                                 //^
const byte PDO_S3 = 25;                                 //^
const byte PAI_Steps = 35;                              //^
const byte PDO_Enable = 32;                             //^ LOW=Multiplexer EBABLED

enum DIRECTIONS {DOWN, UP};
bool Direction = UP;
bool UpdateLEDs = false;
const byte LEDSections = sizeof(Stair) / sizeof(Stair[0]);
byte lastStep = 0;
byte ExtraDirection = 1;
CRGB LEDs[TotalLEDs];
#define LED_TYPE WS2812B
void setup() {
  pinMode(PDO_S0, OUTPUT);
  pinMode(PDO_S1, OUTPUT);
  pinMode(PDO_S2, OUTPUT);
  pinMode(PDO_S3, OUTPUT);
  pinMode(PDO_Enable, OUTPUT);
  digitalWrite(PDO_Enable, LOW);
  pinMode(PAI_Steps, INPUT);
  FastLED.addLeds<LED_TYPE, PAO_LED, GRB>(LEDs, TotalLEDs);
  FastLED.setBrightness(1);                                     //Set start brightness to be amost off
  fill_solid(&(LEDs[0]), TotalLEDs, CRGB(1, 1, 1));
  FastLED.show();
  FastLED.delay(1);
  FastLED.clear();
  fill_solid(&(LEDs[0]), TotalLEDs, LEDColorOff);
  FastLED.show();
  FastLED.setBrightness(255);
}
void loop() {
  static unsigned long LastTime;
  if (TickEveryXms(&LastTime, 1))
    StairDepleteCheck();                                //Deplete the LEDs when needed

  bool UpdateState = UpdateSteps();
  static bool LastUpdateSteps = !UpdateState;
  if (UpdateState == false) {
    if (UpdateState != LastUpdateSteps) {
      fill_solid(&(LEDs[0]), TotalLEDs, LEDColorOff);
      UpdateLEDs = true;
    }
  } else {
    if (UpdateState != LastUpdateSteps) {
      fill_solid(&(LEDs[0]), TotalLEDs, LEDColorIdle);
      UpdateLEDs = true;
    }
    for (byte i = 0; i < LEDSections; i++) {
      StairStepCheck(&Stair[i], i);
    }
  }
  if (UpdateLEDs) {
    UpdateLEDs = false;
    FastLED.show();
  }
  LastUpdateSteps = UpdateState;
}
bool UpdateSteps() {
  //Do we need an LED update?
  bool CountOn = false;
  for (byte i = 0; i < LEDSections; i++) {
    if (StepRead(i) >= TriggerThreshold)
      CountOn = true;
    if (Stair[i].StayOnFor > 0)
      CountOn = true;
  }
  return CountOn;
}
void StairStepCheck(Step *ThisStep, byte _Section) {
  if (StepRead(_Section) < TriggerThreshold) {
    ThisStep->State = false;  // Mark this step as inactive
    return;
  }
  if (ThisStep->State != true) {  // If this step just got pressed
    ThisStep->State = true;
    if (_Section != 0 && _Section < lastStep) {
      Direction = DOWN;
    } else {
      Direction = UP;
    }
    lastStep = _Section;
  }

  int StartSection = _Section - 1;
  if (Direction == DOWN) StartSection = StartSection - ExtraDirection;
  StartSection = constrain(StartSection, 0, LEDSections);

  int EndSection = _Section + 1;
  if (Direction == UP) EndSection = EndSection + ExtraDirection;
  EndSection = constrain(EndSection, 0, LEDSections);

  for (int i = StartSection; i <= EndSection; i++) {
    if (Stair[i].StayOnFor < LEDTimeIdle) {
      Stair[i].StayOnFor = LEDTimeIdle;
      if (LEDs[StartPos(i)] != LEDColorNextOn) {
        fill_solid(&(LEDs[StartPos(i)]), Stair[i].SectionLength, LEDColorNextOn);
        UpdateLEDs = true;
      }
    }
  }
  Stair[_Section].StayOnFor = LEDTimeOn;
  if (LEDs[StartPos(_Section)] != LEDColorOn) {
    fill_solid(&(LEDs[StartPos(_Section)]), Stair[_Section].SectionLength, LEDColorOn);
    UpdateLEDs = true;
  }
  Stair[_Section].State = true;
}
void StairDepleteCheck() {
  for (byte i = 0; i < LEDSections; i++) {
    if (Stair[i].StayOnFor > 0) {
      Stair[i].StayOnFor -= 1;
      if (Stair[i].StayOnFor <= 0) {
        fill_solid(&(LEDs[StartPos(i)]), Stair[i].SectionLength, LEDColorIdle);
        UpdateLEDs = true;
      }
    }
  }
}
int StartPos(byte Section) {
  int Counter = 0;
  for (byte i = 0; i < LEDSections; i++) {
    if (i == Section)
      return Counter;
    Counter += Stair[i].SectionLength;
  }
  return 0;
}
bool TickEveryXms(unsigned long * _LastTime, unsigned long _Delay) {
  static unsigned long _Middle = -1;
  if (_Middle == -1) _Middle = _Middle / 2;
  if (millis() - (*_LastTime + _Delay + 1) < _Middle) {
    *_LastTime = millis();
    return true;
  }
  return false;
}
byte StepRead(byte Channel) {
  //using a CD74HC4067
  digitalWrite(PDO_S0, bitRead(Channel, 0));
  digitalWrite(PDO_S1, bitRead(Channel, 1));
  digitalWrite(PDO_S2, bitRead(Channel, 2));
  digitalWrite(PDO_S3, bitRead(Channel, 3));
  byte ReturnValue = ReadAverage(analogRead(PAI_Steps) / AnalogScaler, &Stair[Channel].Average);
  return ReturnValue;
}
