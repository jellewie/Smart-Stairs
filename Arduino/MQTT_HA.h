#include <ArduinoHA.h>                                          //https://github.com/dawidchyrzynski/arduino-home-assistant/
IPAddress HA_BROKER_ADDR = IPAddress(0, 0, 0, 0);
String HA_BROKER_USERNAME = "";
String HA_BROKER_PASSWORD = "";
#define HA_deviceSoftwareVersion "1.1"                          //Device info - Firmware:
#define HA_deviceManufacturer "JelleWho"                        //Manufacturer
#define HA_deviceModel "Smart-Stair"                            //Model
#define HA_lightName "stairenabled"                             //Entity ID
unsigned long HA_EveryXmsReconnect = 60 * 60 * 1000;            //On which interfall to check if WiFi still works
#define HA_EveryXmsUpdate 60 * 1000                             //How often to send the LDR sensor value to HA
byte mac[] = {0x00, 0x10, 0xFA, 0x6E, 0x38, 0x4C};              //We need to tell HA we are a new device
WiFiClient client;
HADevice device(mac, sizeof(mac));
HAMqtt mqtt(client, device);
HALight HAlight("smartStair");                                  //unique LighID used for the whole LED strip
HASensorNumber HALDR("ldr");                                    //unique SensorNumberID used to send the LDR data to HA
HANumber number("LDRmax");                                      //unique NumberID used to set a trigger setpoint from HA
bool HA_MQTT_Enabled = false;                                   //If MQTT has runned the setup yet
void onStateCommand(bool state, HALight* sender) {
  LEDsEnabled = state;
  sender->setState(state);                                      //Report state back to the Home Assistant
}
void onNumberCommand(HANumeric number, HANumber* sender) {
  if (!number.isSet()) {
    LDRmax = 4096;                                              //The reset command was send by Home Assistant
  } else {
    LDRmax = number.toInt16();
  }
  sender->setState(LDRmax);                                     //Report the selected option back to the HA panel
  HALDR.setValue(ReadLDR());
}
void HaSetup() {
  if (HA_MQTT_Enabled) return;
  if (HA_BROKER_ADDR == IPAddress(0, 0, 0, 0)) return;          //Stop HA MQTT when no IP has been setup
  if (HA_BROKER_USERNAME == "") return;
  if (HA_BROKER_PASSWORD == "") return;
  device.setName(Name);
  device.setSoftwareVersion(HA_deviceSoftwareVersion);
  device.setManufacturer(HA_deviceManufacturer);
  device.setModel(HA_deviceModel);
  // static String URL = "http://" + IpAddress2String(WiFi.localIP());
  // static char configUrl[30];  // Adjust size as needed, large enough to hold the URL
  // URL.toCharArray(configUrl, sizeof(configUrl));
  //device.setConfigurationUrl(configUrl);

  HAlight.setName(HA_lightName);
  HAlight.onStateCommand(onStateCommand);
  HAlight.setIcon("mdi:stairs");

  HALDR.setName("Light");
  HALDR.setUnitOfMeasurement("lx");
  HALDR.setIcon("mdi:brightness-5");

  number.onCommand(onNumberCommand);
  number.setName("LDRmax");
  number.setMin(0);
  number.setMax(255);
  number.setStep(1);
  number.setRetain(true);

  mqtt.begin(HA_BROKER_ADDR, HA_BROKER_USERNAME.c_str(), HA_BROKER_PASSWORD.c_str());
  HA_MQTT_Enabled = true;                                       //Set this before HaLoop to avoid looping
}
void HaLoop() {
  static unsigned long LastTime;
  if (TickEveryXms(&LastTime, HA_EveryXmsReconnect)) {
    if (WiFiManager.CheckAndReconnectIfNeeded(false)) {         //Try to connect to WiFi, but dont start ApMode
      HaSetup();
      HAlight.setState(LEDsEnabled, true);                      //(state, force)
    }
  }
  static int8_t HALastLEDsEnabled = !LEDsEnabled;
  if (HALastLEDsEnabled != LEDsEnabled) {                       //If the HA value is not the same as the current value
    HALastLEDsEnabled = LEDsEnabled;
    HAlight.setState(LEDsEnabled);
  }
  static unsigned long LastTime2 = 0;
  if (TickEveryXms(&LastTime2, HA_EveryXmsUpdate)) {
    HALDR.setValue(ReadLDR());
  }
  WiFiManager.RunServer();
  HaSetup();                                                    //Run setup if we haven't yet
  mqtt.loop();
}
