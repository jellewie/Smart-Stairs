/*
  Program written by JelleWho https://github.com/jellewie/Smart-Stairs
  Board: https://dl.espressif.com/dl/package_esp32_index.json
*/

#if !defined(ESP32)
#error "Please check if the 'DOIT ESP32 DEVKIT V1' board is selected, which can be downloaded at https://dl.espressif.com/dl/package_esp32_index.json"
#endif
#include <FastLED.h>

const byte PDO_S0 = 13;      //Connections to CD74HC4067
const byte PDO_S1 = 27;      //^
const byte PDO_S2 = 33;      //^
const byte PDO_S3 = 25;      //^
const byte PAI_Steps = 35;   //^
const byte PDO_Enable = 32;  //^ LOW=Multiplexer EBABLED
const byte PAI_LDR = 36;     //To which pin the light sensor is attached to
const byte PAO_LED = 5;      //To which pin the LED strip is attached to
struct Step {
  uint8_t SectionLength = 0;  //Amount of LEDs in this secion
  bool State = false;         //The current state, HIGH/LOW this is used for initializing.
  bool StateLast = false;     //Last processed state
  int StayOnFor = 0;          //The time the step should still be light up
  CRGB LEDColor;
};
Step Stair[] = { Step{ 25 },  //The steps and their LED section length
                 Step{ 27 },
                 Step{ 27 },
                 Step{ 26 },
                 Step{ 27 },
                 Step{ 25 },
                 Step{ 24 },
                 Step{ 26 },
                 Step{ 27 },
                 Step{ 26 },
                 Step{ 28 },
                 Step{ 26 },
                 Step{ 26 },
                 Step{ 0 } };
enum DIRECTIONS { DOWN,
                  UP };  //Just Enum this for an easier code reading
bool Direction = UP;     //The direction the user is walking
bool UpdateLEDs = true;  //If the LEDs needs an update
bool TooBright = false;  //If its to bright, disable the stairs
bool StairIsOn = false;
int8_t lastStep = 0;                                          //The last known step that is triggered, used to calculate direction
int8_t TriggerThreshold = 60;                                 //The amount where the step should be considered occupied
int8_t ExtraDirection = 1;                                    //Amount of LEDs to turn on extra in the direction the user is going
int16_t LDRmax = 4095;                                        //Above this light/lux ignore the steps
int16_t LEDTimeOn = 750;                                      //The time in ms that would set the minimum on-time of the step
int16_t LEDTimeIdle = 500;                                    //The time in ms that would set the minimum idle-time of the step (not additive!)
const int8_t LEDSections = sizeof(Stair) / sizeof(Stair[0]);  //The amount of steps
const int16_t TotalLEDs = LEDSections * 30;                   //The amount of LEDs in the stair
CRGB LEDs[TotalLEDs];
CRGB LEDColorOn = { 5, 2, 2 };      //The color of the steps in set state
CRGB LEDColorNextOn = { 2, 1, 1 };  //^
CRGB LEDColorOff = { 0, 0, 0 };     //^
CRGB LEDColorIdle = { 1, 0, 0 };    //^

void setup() {
  pinMode(PDO_S0, OUTPUT);
  pinMode(PDO_S1, OUTPUT);
  pinMode(PDO_S2, OUTPUT);
  pinMode(PDO_S3, OUTPUT);
  pinMode(PDO_Enable, OUTPUT);
  digitalWrite(PDO_Enable, LOW);
  pinMode(PAI_Steps, INPUT);
  FastLED.addLeds<WS2812B, PAO_LED, GRB>(LEDs, TotalLEDs);
  FastLED.setBrightness(255);
  LED_Fill(0, TotalLEDs, LEDColorIdle);  //Blink on boot, so show the LEDs are working
  FastLED.show();                      //Update LEDs
  delay(100);
  LED_Fill(0, TotalLEDs, LEDColorOff);
  FastLED.show();  //Update LEDs
}

void loop() {
  StairDepleteCheck();  //Deplete the LEDs when needed
  UpdateSteps();
  if (StairIsOn == false)                             //If the stair is off
    TooBright = (ReadLDR() > LDRmax) ? true : false;  //Update the TooBright state, this prevents the state from changing due to its own light
  if (TooBright == false) {                           //Only allow stairs mode, if it was dark when we started
    for (byte i = 0; i < LEDSections; i++) {          //For each step
      StairStepCheck(&Stair[i], i);                   //Check if a step is triggered
    }
  }
  if (UpdateLEDs) {                             //If the LEDs need an update
    if (StairIsOn) {                            //If someone is on the stairs
      LED_Fill(0, TotalLEDs, LEDColorIdle);     //Base color all LEDs as idle
      for (byte i = 0; i < LEDSections; i++) {  //For each step
        if (Stair[i].State)
          LED_Fill(StartPos(i), Stair[i].SectionLength, Stair[i].LEDColor);
      }
    } else {
      LED_Fill(0, TotalLEDs, LEDColorOff);  //Turn LEDs off
    }
    FastLED.show();  //Update LEDs
    UpdateLEDs = false;
  }
}
bool UpdateSteps() {  //Is a step pressed?
  bool ReturnValue = false;
  for (byte i = 0; i < LEDSections; i++) {  //For each step
    if (StepRead(i) >= TriggerThreshold)    //If someone is standing on this step
      Stair[i].State = true;
    else
      Stair[i].State = false;                                       //Mark this step as inactive
    if (StepRead(i) >= TriggerThreshold || Stair[i].StayOnFor > 0)  //If an section is on, or needs to stay on for a bit longer
      ReturnValue = true;
  }
  StairIsOn = ReturnValue;
  return ReturnValue;
}
uint8_t StepRead(uint8_t _Section) {      //Return if a step is occupied, we are using a CD74HC4067 here to extend IO
  if (_Section >= LEDSections) return 0;  //(Protection out of array bounds)
  digitalWrite(PDO_S0, bitRead(_Section, 0));
  digitalWrite(PDO_S1, bitRead(_Section, 1));
  digitalWrite(PDO_S2, bitRead(_Section, 2));
  digitalWrite(PDO_S3, bitRead(_Section, 3));
  const uint8_t AnalogScaler = 16;  //Since the ESP has an 12bit analog, but we use an 8bit, set the conversion factor here. pow(2, (12 - 8))
  uint8_t ReturnValue = analogRead(PAI_Steps) / AnalogScaler;
  return ReturnValue;
}
int16_t ReadLDR() {
  uint16_t ReturnValue = 4095 - analogRead(PAI_LDR);  //Inverse so dark=0 and bright=4096
  return ReturnValue;
}
void StairDepleteCheck() {  //Deplete STAY-ON counters of all steps
  EVERY_N_MILLISECONDS(1) {
    for (byte i = 0; i < LEDSections; i++) {  //For each step
      if (Stair[i].StayOnFor > 0) {           //If it needs to stay on
        Stair[i].StayOnFor -= 1;              //Deplete it a bit more
        if (Stair[i].StayOnFor == 0)
          UpdateLEDs = true;
      }
    }
  }
}
void StairStepCheck(Step* ThisStep, byte _Section) {
  if (_Section >= LEDSections) return;  //(Protection out of array bounds)
  bool JustUpdated = Stair[_Section].StateLast != Stair[_Section].State;
  Stair[_Section].StateLast = Stair[_Section].State;
  if (Stair[_Section].State == false) return;      //If step is not pressed
  if (JustUpdated) {                               //If this step just got pressed
    if ((_Section != 0 && _Section < lastStep)) {  //If we go downstairs
      Direction = DOWN;
    } else {
      Direction = UP;
    }
    lastStep = _Section;
  }
  int8_t StartSection = _Section - 1;
  if (Direction == DOWN) StartSection = StartSection - ExtraDirection;
  StartSection = constrain(StartSection, 0, LEDSections - 1);

  int8_t EndSection = _Section + 1;
  if (Direction == UP) EndSection = EndSection + ExtraDirection;
  EndSection = constrain(EndSection, 0, LEDSections - 1);

  for (int8_t i = StartSection; i <= EndSection; i++) {  //For one step behind the user, and 1+extra steps before the user
    if (Stair[i].StayOnFor < LEDTimeIdle) {              //If we are in idle time territory
      Stair[i].StayOnFor = LEDTimeIdle;                  //(re)set the timer on how long the LEDs need to stay idle
      if (Stair[i].LEDColor != LEDColorNextOn) {         //If needed to update color
        Stair[i].LEDColor = LEDColorNextOn;
        UpdateLEDs = true;  //Flag that we need to update the LEDs
      }
    }
  }
  Stair[_Section].StayOnFor = LEDTimeOn;         //(re)set the timer on how long the LEDs need to stay on
  if (Stair[_Section].LEDColor != LEDColorOn) {  //If needed to update color
    Stair[_Section].LEDColor = LEDColorOn;
    UpdateLEDs = true;  //Flag that we need to update the LEDs
  }
  Stair[_Section].State = true;
}
int StartPos(byte Section) {  //Returns the position of the first LED of the section
                              //We could cache these values, but for now to keep it simple we compute them each time
  if (Section >= LEDSections) return 0;
  int Counter = 0;
  for (byte i = 0; i < LEDSections; i++) {
    if (i == Section)
      return Counter;
    Counter += Stair[i].SectionLength;
  }
  return 0;  //Section error, just return 0
}
void LED_Fill(int From, int Amount, CRGB Color) {
  if (From < 0 || Amount <= 0 || From >= TotalLEDs) return;  //(Protection out of array bounds) fully out of bounds
  if (From + Amount > TotalLEDs) return;                     //(Protection out of array bounds) partially out of bounds
  fill_solid(&(LEDs[From]), Amount, Color);
}