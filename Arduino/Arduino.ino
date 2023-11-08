#include <FastLED.h>
#define AverageAmount 8                                         //The amount of analog measurements to take an average from for the step sensor
#define AnalogScaler pow(2,(12 - 8))                            //Since the ESP has an 10bit analog, but we use an 8bit, set the conversion factor here

struct AVG {
  byte Counter;                                                 //Where we are in the array
  long PointTotal;                                              //The sum of all values in the array
  byte Point[AverageAmount];                                    //The array of all values
};
struct Step {
  byte SectionLength;                                           //Amount of LEDs in this secion
  bool State;                                                   //The current state, HIGH/LOW this is used for initializing
  int StayOnFor;                                                //The time the step should still be light up
  AVG Average;                                                  //The average analog value of the step
};
byte ReadAverage(byte Input, AVG *av) {                         //Returns the average of the last AverageAmount measurements.
  av->PointTotal -= av->Point[av->Counter];                     //Remove the old value from the total value
  av->Point[av->Counter] = Input;                               //Add the new number into the array
  av->PointTotal += av->Point[av->Counter];                     //Add the new value to the total value
  av->Counter++;                                                //Increase the counter
  if (av->Counter >= AverageAmount)                             //If the counter is at its configured end
    av->Counter = 0;                                            //Go back down
  byte ReturnValue = av->PointTotal / AverageAmount;            //Calculate the average
  return ReturnValue;
}
Step Stair[] = {Step{25},                                       //The steps and their LED section length
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
const byte TriggerThreshold = 60;                               //The amount where the step should be considered occupied
const int LEDTimeOn = 400;                                      //The time in ms that would set the minimum on-time of the step
const int LEDTimeIdle = 200;                                    //The time in ms that would set the minimum idle-time of the step (not additive!)
CRGB LEDColorOn = {5, 2, 2};                                    //The color of the steps in set state
CRGB LEDColorNextOn = {2, 1, 1};                                //^
CRGB LEDColorOff = {0, 0, 0};                                   //^
CRGB LEDColorIdle = {1, 0, 0};                                  //^
const byte ExtraDirection = 1;                                  //Amount of LEDs to turn on extra in the direction the user is going
const byte PAO_LED = 5;                                         //To which pin the LED strip is attached to
const byte LEDSections = sizeof(Stair) / sizeof(Stair[0]);      //The amount of steps
const int TotalLEDs = LEDSections * 28;                         //The amount of LEDs in the stair
const byte PDO_S0 = 13;                                         //Connections to CD74HC4067
const byte PDO_S1 = 27;                                         //^
const byte PDO_S2 = 33;                                         //^
const byte PDO_S3 = 25;                                         //^
const byte PAI_Steps = 35;                                      //^
const byte PDO_Enable = 32;                                     //^ LOW=Multiplexer EBABLED
enum DIRECTIONS {DOWN, UP};                                     //Just Enum this for an easier code reading
bool Direction = UP;                                            //The direction the user is walking
bool UpdateLEDs = true;                                         //If the LEDs needs an update
byte lastStep = 0;                                              //The last known step that is triggered, used to calculate direction
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
  fill_solid(&(LEDs[0]), TotalLEDs, LEDColorOff);
  FastLED.setBrightness(255);
}
void loop() {
  static unsigned long LastTime;
  if (TickEveryXms(&LastTime, 1))
    StairDepleteCheck();                                        //Deplete the LEDs when needed

  bool UpdateState = UpdateSteps();
  static bool LastUpdateSteps = !UpdateState;
  if (UpdateState == false) {                                   //Are we OFF?
    if (UpdateState != LastUpdateSteps) {                       //Is there an update?
      fill_solid(&(LEDs[0]), TotalLEDs, LEDColorOff);           //Turn LEDs off
      UpdateLEDs = true;                                        //Flag that we need to update the LEDs
    }
  } else {
    if (UpdateState != LastUpdateSteps) {                       //Is there an update?
      fill_solid(&(LEDs[0]), TotalLEDs, LEDColorIdle);          //Base color all LEDs as idle
      UpdateLEDs = true;                                        //Flag that we need to update the LEDs
    }
    for (byte i = 0; i < LEDSections; i++) {                    //For each step
      StairStepCheck(&Stair[i], i);
    }
  }
  if (UpdateLEDs) {                                             //If the LEDs need an update
    FastLED.show();                                             //Update LEDs
    UpdateLEDs = false;                                         //Flag update as complete
  }
  LastUpdateSteps = UpdateState;                                //Remenber what this step state was for next time
}
bool UpdateSteps() {                                            //Do we need an LED update?
  for (byte i = 0; i < LEDSections; i++) {
    if (StepRead(i) >= TriggerThreshold or Stair[i].StayOnFor > 0) //If an section is on, or needs to stay on for a bit longer
      return true;
  }
  return false;
}
void StairStepCheck(Step *ThisStep, byte _Section) {
  if (StepRead(_Section) < TriggerThreshold) {
    ThisStep->State = false;                                    //Mark this step as inactive
    return;
  }
  if (ThisStep->State != true) {                                //If this step just got pressed
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

  for (int i = StartSection; i <= EndSection; i++) {            //For one step behind the user, and 1+extra steps before the user
    if (Stair[i].StayOnFor < LEDTimeIdle) {                     //If we are in idle time territory
      Stair[i].StayOnFor = LEDTimeIdle;                         //(re)set the timer on how long the LEDs need to stay idle
      if (LEDs[StartPos(i)] != LEDColorNextOn) {                //If needed to update color
        fill_solid(&(LEDs[StartPos(i)]), Stair[i].SectionLength, LEDColorNextOn);
        UpdateLEDs = true;                                      //Flag that we need to update the LEDs
      }
    }
  }
  Stair[_Section].StayOnFor = LEDTimeOn;                        //(re)set the timer on how long the LEDs need to stay on
  if (LEDs[StartPos(_Section)] != LEDColorOn) {                 //If needed to update color
    fill_solid(&(LEDs[StartPos(_Section)]), Stair[_Section].SectionLength, LEDColorOn);
    UpdateLEDs = true;                                          //Flag that we need to update the LEDs
  }
  Stair[_Section].State = true;
}
void StairDepleteCheck() {                                      //Deplete STAY-ON counters of all steps
  for (byte i = 0; i < LEDSections; i++) {                      //For each step
    if (Stair[i].StayOnFor > 0) {                               //If it needs to stay on
      Stair[i].StayOnFor -= 1;                                  //Deplete it a bit more
      if (Stair[i].StayOnFor <= 0) {                            //If we hit the bottum
        fill_solid(&(LEDs[StartPos(i)]), Stair[i].SectionLength, LEDColorIdle); //Turn it to idle color
        UpdateLEDs = true;                                      //Flag that we need to update the LEDs
      }
    }
  }
}
int StartPos(byte Section) {                                    //Returns the position of the first LED of the section
  int Counter = 0;
  for (byte i = 0; i < LEDSections; i++) {
    if (i == Section)
      return Counter;
    Counter += Stair[i].SectionLength;
  }
  return 0;                                                    //Section error, just return 0
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
byte StepRead(byte Channel) {                                   //Return if a step is occupied, we are using a CD74HC4067 here to extend IO
  digitalWrite(PDO_S0, bitRead(Channel, 0));
  digitalWrite(PDO_S1, bitRead(Channel, 1));
  digitalWrite(PDO_S2, bitRead(Channel, 2));
  digitalWrite(PDO_S3, bitRead(Channel, 3));
  byte ReturnValue = ReadAverage(analogRead(PAI_Steps) / AnalogScaler, &Stair[Channel].Average);
  return ReturnValue;
}
