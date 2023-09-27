/*
  Program written by JelleWho https://github.com/jellewie

  https://wiki.dfrobot.com/Thin_Film_Pressure_Sensor_SKU_SEN0293_SEN0294_SEN0295_SEN0296_SEN0297_SEN0298_SEN0299
  SEN0299

  y=153.1\cdot x^{-0.69} https://www.desmos.com/calculator/0lmmwgbxx4 (Y=Kilogram,X=Ohms)

*/
#if !defined(ESP32)
#error "Please check if the 'DOIT ESP32 DEVKIT V1' board is selected, which can be downloaded at https://dl.espressif.com/dl/package_esp32_index.json"
#endif
//==============================================================//Note spacer

//#define DEBUG_MODE_LED

#include <FastLED.h>                                            //Include the libary FastLED (If you get a error here, make sure it's installed!)
#define StableAnalog_AverageAmount 8                            //On how much points to take the average from (default=16)
#define StableAnalog_AnalogResolution 12                        //Howmany bits an analog read is (default=8 same as default Arduino boards)
#include "StableAnalog/StableAnalog.h"
struct Stair {
  StableAnalog SA;
  byte SectionLength;
};
/*
  "ADC2 pins cannot be used when Wi-Fi is used"
  ADC1_CH0 (GPIO 36) (input only)
  ADC1_CH3 (GPIO 39) (input only)
  ADC1_CH6 (GPIO 34) (input only)
  ADC1_CH7 (GPIO 35) (input only)
  ADC1_CH4 (GPIO 32)
  ADC1_CH5 (GPIO 33)
  ADC2_CH8 (GPIO 25)
  ADC2_CH9 (GPIO 26)
  ADC2_CH7 (GPIO 27)
  ADC2_CH6 (GPIO 14) (outputs PWM signal at boot)
  ADC2_CH5 (GPIO 12) (boot fail if pulled high)
  ADC2_CH4 (GPIO 13)

  ADC2_CH0 (GPIO 4)
  ADC2_CH2 (GPIO 2)  (connected to on-board LED)

  ADC2_CH1 (GPIO 0)  (outputs PWM signal at boot)

  These are not connected/do not exist
  ADC1_CH1 (GPIO 37)
  ADC1_CH2 (GPIO 38)

  IRF3808PBF can switch about 2300 LEDs on/off. Just need to test if 5V gate voltage works
*/
//==============================================================
//USER SETTINGS
//==============================================================
//StableAnalog(<PIN>),<SectionLength>
Stair Sence[] = {Stair{StableAnalog(36), 0},                    //ADC1_CH0 (GPIO 36) (input only)
                 Stair{StableAnalog(39), 25},                   //ADC1_CH3 (GPIO 39) (input only)
                 Stair{StableAnalog(34), 27},                   //ADC1_CH6 (GPIO 34) (input only)
                 Stair{StableAnalog(35), 27},                   //ADC1_CH7 (GPIO 35) (input only)
                 Stair{StableAnalog(32), 26},
                 Stair{StableAnalog(33), 27},
                 Stair{StableAnalog(25), 25},
                 Stair{StableAnalog(26), 24},
                 Stair{StableAnalog(27), 26},
                 Stair{StableAnalog(14), 28},                   //ADC2_CH6 (GPIO 14) (outputs PWM signal at boot)
                 Stair{StableAnalog(13), 25},

                 Stair{StableAnalog( 4), 25},
                 Stair{StableAnalog(15), 25}                    //ADC2_CH3 (GPIO 15) (outputs PWM signal at boot)
                };
const byte PAO_LED = 23;                                        //To which pin the <LED strip> is connected to
const int TotalLEDs = 13 * 30;                                  //The total amounts of LEDs in the strip
const byte PotMinChange = 2;                                    //Howmuch the pot_value needs to change before we process it (default=2)
const byte PotStick = 2;                                        //If this close to HIGH(max) or LOW(min) stick to it (default=2)
const byte TriggerThreshold = 60;                               //When a value should be considered HIGH
const int LEDTimeOn = 500;                                      //The minum time an active section stays on

CRGB LEDColorOn = {10, 5, 5};                                   //The color of thr triggered section
CRGB LEDColorNextOn = {2, 1, 1};                                //The color of the adjacent to triggered sections
CRGB LEDColorOff = {0, 0, 0};
CRGB LEDColorIdle = {1, 0, 0};                                  //The color when at least one step is active

//==============================================================
//END OF USER SETTINGS
//==============================================================
const byte LEDSections = sizeof(Sence) / sizeof(Sence[0]);      //Amount of sections
POT StepStatus[LEDSections];
enum E_Dir {DOWN, UP};
bool Direction = UP;
bool UpdateLEDs = false;
byte lastStep = 0;
byte ExtraDirection = 1;                                        //Amount of extra steps to light in the direction the user is going
bool StepStates[LEDSections] = {0};
int StayOnCount[LEDSections] = {0};                             //Used to hold the sections that need to stay on, values are time to stay on in mS
#define LED_TYPE WS2812B                                        //WS2812B for 5V LEDs
CRGB LEDs[TotalLEDs];
void setup() {
  Serial.begin(115200);
#ifdef DEBUG_MODE_LED
  Serial.println("Starting in DEBUG_MODE_LED");
#endif
  FastLED.addLeds<LED_TYPE, PAO_LED, GRB>(LEDs, TotalLEDs);     //Init LEDs
  fill_solid(&(LEDs[0]), TotalLEDs, CRGB(1, 1, 1));             //Blink on boot
  FastLED.show();
  FastLED.delay(1);
  FastLED.clear();
  fill_solid(&(LEDs[0]), TotalLEDs, LEDColorOff);
  FastLED.show();
  FastLED.delay(100);
}

void loop() {
#ifdef DEBUG_MODE_LED
  for (byte i = 0; i < LEDSections; i++) {
    Serial.println("DB: doing section " + String(i) + " S=" + String(StartPos(i)) + " L=" + String(Sence[i].SectionLength));
    fill_solid(&LEDs[0], TotalLEDs, LEDColorOff);
    if (i > 0) fill_solid(&(LEDs[StartPos(i - 1)]), Sence[i - 1].SectionLength, LEDColorNextOn);                //Turn On the LED section before this one
    fill_solid(&(LEDs[StartPos(i)]), Sence[i].SectionLength, LEDColorOn);                                       //Turn On the LED section
    if (i < LEDSections - 1) fill_solid(&(LEDs[StartPos(i + 1)]), Sence[i + 1].SectionLength, LEDColorNextOn);  //Turn On the LED section after this one
    FastLED.show();
    FastLED.delay(500);
  }
#else
  static unsigned long LastTime;
  if (TickEveryXms(&LastTime, 1))                               //Run every 1ms
    StairDepleteCheck();                                        //Counts down the timer, and turns LEDs off when needed
  
  bool UpdateState = UpdateSteps();
  static bool LastUpdateSteps = not UpdateState;
  if (UpdateState == 0) {
    if (UpdateState != LastUpdateSteps) {
      fill_solid(&(LEDs[0]), TotalLEDs, LEDColorOff);
      UpdateLEDs = true;
    }
  } else {
    fill_solid(&(LEDs[0]), TotalLEDs, LEDColorIdle);
    for (byte i = 0; i < LEDSections; i++) {
      StairStepCheck(i, StepStatus[i]);                           //Check if any LED section needs to be on
    }
  }
  if (UpdateLEDs) {
    UpdateLEDs = false;
    FastLED.show();                                             //Update
  }
  LastUpdateSteps = UpdateState;
#endif
}
bool UpdateSteps() {
  byte CountOn = false;
  //Serial.print(String(Device.Value) + " ");
  for (byte i = 0; i < LEDSections; i++) {
    StepStatus[i] = Sence[i].SA.ReadStable();
    if (StepStatus[i].Value >= TriggerThreshold)    //If its triggered
      CountOn = true;
    if (StayOnCount[i] > 0)                         //If its still decaying
      CountOn = true;
  }
  //Serial.println("255");
  return CountOn;
}
void StairStepCheck(byte _Section, POT Device) {
  //Make it so the "_Section" will be on for at least "LEDTimeOn" time, and "ExtraDirection" above or below that will be on half that time

  if (Device.Value < TriggerThreshold) {                       //If we do not sense something
    StepStates[_Section] = false;                              //Remember we have not processed this step
    return;
  }
  if (StepStates[_Section] != true) {
    if (_Section != 0 and _Section < lastStep) {
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


  if (_Section == 3) {
    Serial.print("Test i=");
    Serial.print(_Section);
    Serial.print(" d=");
    Serial.print(Direction);
    Serial.print(" s=");
    Serial.print(StartSection);
    Serial.print(" e=");
    Serial.print(EndSection);
    Serial.print(" t=");
    Serial.print(StayOnCount[_Section]);

    Serial.print(" sp 1=" + String(LEDs[StartPos(1)].r));
    Serial.print(" 2=" + String(LEDs[StartPos(2)].r));
    Serial.print(" 3=" + String(LEDs[StartPos(3)].r));
    Serial.print(" 4=" + String(LEDs[StartPos(4)].r));
    Serial.print(" 5=" + String(LEDs[StartPos(5)].r));
    Serial.print(" 6=" + String(LEDs[StartPos(6)].r));

    //Test i=3 d=0 s=1 e=4 t=499
  }

  for (int s = StartSection; s <= EndSection; s++) {
    if (StayOnCount[s] < LEDTimeOn / 2) {
      StayOnCount[s] = LEDTimeOn / 2;
      if (LEDs[StartPos(s)] != LEDColorNextOn or LEDs[StartPos(s)] != LEDColorOn) { //If this part is not on yet
        if (_Section == 3) Serial.print(" triggered");
        fill_solid(&(LEDs[StartPos(s)]), Sence[s].SectionLength, LEDColorNextOn);   //Turn On the LED section before this one
        UpdateLEDs = true;
      }
    }
  }

  if (_Section == 3) Serial.println();

  StayOnCount[_Section] = LEDTimeOn;
  if (LEDs[StartPos(_Section)] != LEDColorOn) {               //If this part is not completly on yet
    fill_solid(&(LEDs[StartPos(_Section)]), Sence[_Section].SectionLength, LEDColorOn); //Turn On the LED section
    UpdateLEDs = true;
  }
  StepStates[_Section] = true;                                //Remember we have processed this step
}
void StairDepleteCheck() {
  //Decreases the on timer, and turns the LEDs off when it reaches 0
  for (byte i = 0; i < LEDSections; i++) {
    if (StayOnCount[i] > 0) {                                 //If the LED section is turned on
      StayOnCount[i] -= 1;                                    //Decrease the counter
      if (StayOnCount[i] <= 0) {                              //If the counter has now depleted
        fill_solid(&(LEDs[StartPos(i)]), Sence[i].SectionLength, LEDColorIdle); //Turn that LED section idle
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
    Counter += Sence[i].SectionLength;
  }
  return 0;
}
bool TickEveryXms(unsigned long * _LastTime, unsigned long _Delay) {
  //With overflow, can be adjusted, no overshoot correction, true when (Now < _LastTime + _Delay)
  /* Example:   static unsigned long LastTime;
                if (TickEveryXms(&LastTime, 1000)) {//Code to run}    */
  static unsigned long _Middle = -1;                            //We just need a really big number, if more than 0 and less than this amount of ms is passed, return true)
  if (_Middle == -1) _Middle = _Middle / 2;                     //Somehow declairing middle on 1 line does not work
  if (millis() - (*_LastTime + _Delay + 1) < _Middle) {         //If it's time to update (keep brackets for overflow protection). If diffrence between Now (millis) and Nextupdate (*_LastTime + _Delay) is smaller than a big number (_Middle) then update. Note that all negative values loop around and will be really big (for example -1 = 4,294,967,295)
    *_LastTime = millis();                                      //Set new LastTime updated
    return true;
  }
  return false;
}
