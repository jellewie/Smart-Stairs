/*
  Program written by JelleWho https://github.com/jellewie/Smart-Stairs
  Board: https://dl.espressif.com/dl/package_esp32_index.json
*/

#if !defined(ESP32)
#error "Please check if the 'DOIT ESP32 DEVKIT V1' board is selected, which can be downloaded at https://dl.espressif.com/dl/package_esp32_index.json"
#endif

#include <WiFi.h>
#include <FastLED.h>
#include "WiFiManagerBefore.h"                                  //Define what options to use/include or to hook into WiFiManager
#include "WiFiManager/WiFiManager.h"                            //Includes <WiFi> and <WebServer.h> and setups up 'WebServer server(80)' if needed
#define WiFiManager_OTA                                         //Define if you want to use the Over The Air update page (/ota)
#define AverageAmount 8                                         //The amount of analog measurements to take an average from for the step sensor
const byte PDO_S0 = 13;                                         //Connections to CD74HC4067
const byte PDO_S1 = 27;                                         //^
const byte PDO_S2 = 33;                                         //^
const byte PDO_S3 = 25;                                         //^
const byte PAI_Steps = 35;                                      //^
const byte PDO_Enable = 32;                                     //^ LOW=Multiplexer EBABLED
const byte PAI_LDR = 36;
#include "functions.h"

Step Stair[16] = {Step{0},                                     //The steps and their LED section length
                  Step{0},
                  Step{0},
                  Step{0},
                  Step{0},
                  Step{0},
                  Step{0},
                  Step{0},
                  Step{0},
                  Step{0},
                  Step{0},
                  Step{0},
                  Step{0},
                  Step{0},
                  Step{0},
                  Step{0}
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
const int TotalLEDs = LEDSections * 50;                         //The amount of LEDs in the stair
enum DIRECTIONS {DOWN, UP};                                     //Just Enum this for an easier code reading
bool Direction = UP;                                            //The direction the user is walking
bool UpdateLEDs = true;                                         //If the LEDs needs an update
bool TooBright = false;
byte lastStep = 0;                                              //The last known step that is triggered, used to calculate direction
int16_t LDRmax = 4096;                                          //Above this light/lux ignore the steps
CRGB LEDs[TotalLEDs];
int8_t Mode = STAIRS;                                           //Set the bootmode to be ON (STAIRS)
int8_t LastMode = -1;
#define LED_TYPE WS2812B
#include "MQTT_HA.h" 
#include "WiFiManagerLater.h"
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
  server.on("/info",        handle_Info);
  server.onNotFound(        handle_NotFound);                   //When a client requests an unknown URI
  WiFiManager.Start();                                          //Run the wifi startup (and save results)
  WiFiManager.EnableSetup(true);                                //(runtime) Enable the settings, only enabled in APmode by default
  WiFiManager.OTA_Enabled = true;
}
void loop() {
  HaLoop();
  ReadLDR();                                                    //Keep the value updated
  switch (Mode) {
    case OFF:{
        if (Mode != LastMode) {                                 //If we just are just DISABLED
          for (byte i = 0; i < LEDSections; i++)                //For each step
            Stair[i].StayOnFor = 0;                             //Deplete it completly
          FastLED.clear();
          UpdateLEDs = true;
        }
    } break;
    case STAIRS: {
        if (StairIsOn() == false)                               //If the stair is off
          TooBright = (ReadLDR() > LDRmax) ? true : false;      //Update the TooBright state, this prevents the state from changing due to its own light
        if (TooBright == false) {
          static unsigned long LastTime;
          if (TickEveryXms(&LastTime, 1))
            StairDepleteCheck();                                //Deplete the LEDs when needed
          bool UpdateState = UpdateSteps();
          static bool LastUpdateSteps = !UpdateState;
          if (UpdateState == false) {                           //Are we OFF?
            if (UpdateState != LastUpdateSteps) {               //Is there an update?
              fill_solid(&(LEDs[0]), TotalLEDs, LEDColorOff);   //Turn LEDs off
              UpdateLEDs = true;                                //Flag that we need to update the LEDs
            }
          } else {
            if (UpdateState != LastUpdateSteps) {               //Is there an update?
              fill_solid(&(LEDs[0]), TotalLEDs, LEDColorIdle);  //Base color all LEDs as idle
              UpdateLEDs = true;                                //Flag that we need to update the LEDs
            }
            for (byte i = 0; i < LEDSections; i++) {            //For each step
              StairStepCheck(&Stair[i], i);                     //Check if a step is triggered
            }
          }
          LastUpdateSteps = UpdateState;                        //Remember what this step state was for next time
        }
      } break;
  }
  LastMode = Mode;
  if (UpdateLEDs) {                                             //If the LEDs need an update
    FastLED.show();                                             //Update LEDs
    UpdateLEDs = false;                                         //Flag update as complete
  }
}
bool UpdateSteps() {                                            //Do we need an LED update?
  HA_StepDetected = true;                                       //Set default to true, will be overwritten
  for (byte i = 0; i < LEDSections; i++) {
    if (StepRead(i) >= TriggerThreshold or Stair[i].StayOnFor > 0) //If an section is on, or needs to stay on for a bit longer
      return true;
  }
  HA_StepDetected = false;
  return false;
}
void StairStepCheck(Step *ThisStep, byte _Section) {
  if (StepRead(_Section) < TriggerThreshold) {
    ThisStep->State = false;                                    //Mark this step as inactive
    return;
  }
  if (ThisStep->State != true) {                                //If this step just got pressed
    ThisStep->State = true;
    if ((_Section != 0 && _Section < lastStep) or (_Section == LEDSections - 1)) {
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

  for (int8_t i = StartSection; i <= EndSection; i++) {         //For one step behind the user, and 1+extra steps before the user
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
bool StairIsOn() {                                              //Check if the stair is currently on
  for (byte i = 0; i < LEDSections; i++) {                      //For each step
    if (Stair[i].StayOnFor > 0) {                               //If it is on
      return true;
    }
  }
  return false;
}
int StartPos(byte Section) {                                    //Returns the position of the first LED of the section
  if (Section >= LEDSections) return 0;
  int Counter = 0;
  for (byte i = 0; i < LEDSections; i++) {
    if (i == Section)
      return Counter;
    Counter += Stair[i].SectionLength;
  }
  return 0;                                                     //Section error, just return 0
}
byte StepRead(byte Channel) {                                   //Return if a step is occupied, we are using a CD74HC4067 here to extend IO
  if (Channel >= LEDSections) return 0;
  digitalWrite(PDO_S0, bitRead(Channel, 0));
  digitalWrite(PDO_S1, bitRead(Channel, 1));
  digitalWrite(PDO_S2, bitRead(Channel, 2));
  digitalWrite(PDO_S3, bitRead(Channel, 3));
#define AnalogScaler pow(2,(12 - 8))                            //Since the ESP has an 10bit analog, but we use an 8bit, set the conversion factor here
  byte ReturnValue = ReadAverage(analogRead(PAI_Steps) / AnalogScaler, &Stair[Channel].Average);
  return ReturnValue;
}
int16_t ReadLDR() {
  static AVG LDR_Average = {};
  return 4096 - ReadAverage(analogRead(PAI_LDR), &LDR_Average); //Inverse so dark=0 and bright=4096
}