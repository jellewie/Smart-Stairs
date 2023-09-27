#include <FastLED.h>
#define StableAnalog_AverageAmount 8
#define StableAnalog_AnalogResolution 12
#include "StableAnalog/StableAnalog.h"
struct Step {
  StableAnalog SA;
  byte SectionLength;
  bool State;
  int StayOnFor;
  POT StepStatus;
};
Step Stair[] = {Step{StableAnalog(39), 25},
                Step{StableAnalog(34), 27},
                Step{StableAnalog(35), 27},
                Step{StableAnalog(32), 26},
                Step{StableAnalog(33), 27},
                Step{StableAnalog(25), 25},
                Step{StableAnalog(26), 24},
                Step{StableAnalog(27), 26},
                Step{StableAnalog(14), 27},
                Step{StableAnalog(13), 26},
                Step{StableAnalog( 4), 28},
                Step{StableAnalog(15), 26},
                Step{StableAnalog(36), 10}
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
const byte PAO_LED = 23;
const int TotalLEDs = 13 * 30;

enum DIRECTIONS {DOWN, UP};
bool Direction = UP;
bool UpdateLEDs = false;
const byte LEDSections = sizeof(Stair) / sizeof(Stair[0]);
byte lastStep = 0;
byte ExtraDirection = 1;
CRGB LEDs[TotalLEDs];
#define LED_TYPE WS2812B
void setup() {

  Serial.begin(115200);

  FastLED.addLeds<LED_TYPE, PAO_LED, GRB>(LEDs, TotalLEDs);
  fill_solid(&(LEDs[0]), TotalLEDs, CRGB(1, 1, 1));
  FastLED.show();
  FastLED.delay(1);
  FastLED.clear();
  fill_solid(&(LEDs[0]), TotalLEDs, LEDColorOff);
  FastLED.show();
  Serial.println("Booted");
}
void loop() {
  static unsigned long LastTime;
  if (TickEveryXms(&LastTime, 1))
    StairDepleteCheck();

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
      StairStepCheck(Stair[i], i);
    }
  }
  if (UpdateLEDs) {
    UpdateLEDs = false;
    FastLED.show();
  }
  LastUpdateSteps = UpdateState;
}
bool UpdateSteps() {
  bool CountOn = false;
  for (byte i = 0; i < LEDSections; i++) {
    Stair[i].StepStatus = Stair[i].SA.ReadStable();
    if (Stair[i].StepStatus.Value >= TriggerThreshold)
      CountOn = true;
    if (Stair[i].StayOnFor > 0)
      CountOn = true;
  }
  return CountOn;
}
void StairStepCheck(Step ThisStep, byte _Section) {
  if (ThisStep.StepStatus.Value < TriggerThreshold) {
    ThisStep.State = false;
    return;
  }
  if (ThisStep.State != true) {
    Serial.println("St=" + String(_Section));
    ThisStep.State = true;
    if (_Section != 0 and _Section < lastStep) {
      Serial.println("DOWN");
      Direction = DOWN;
    } else {
      Direction = UP;
      Serial.println("UP");
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
