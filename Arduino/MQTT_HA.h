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
HASelect HAMode("Mode");                                        //Dropdown menu to select mode
HASensorNumber HALDR("ldr");                                    //unique SensorNumberID used to send the LDR data to HA
HANumber HALDRMax("LDRmax");                                    //unique NumberID used to set a trigger setpoint from HA
extern int16_t ReadLDR();                                       //Declaired later in Arduino.ino
bool HA_MQTT_Enabled = false;                                   //If MQTT has runned the setup yet
void onModeCommand(int8_t index, HASelect* sender) {
  if (index < 0 or index >= Modes_Amount)                       //Sanity check
    return;
  LastMode = -1;                                                //Make sure we init the new mode
  Mode = index;
  sender->setState(index);                                      //report the selected option back to the HA
}
void onNumberCommand(HANumeric number, HANumber* sender) {
  if (!number.isSet()) {
    LDRmax = 4096;                                              //The reset command was send by Home Assistant
  } else {
    LDRmax = number.toInt16();
  }
  sender->setState(LDRmax);                                     //Report the selected option back to the HA panel
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

  String AvailableModes;
  for (size_t i = 0; i < Modes_Amount; i++) {
    if (i > 0) {
        AvailableModes += ';';
    }
    AvailableModes += ModesString[i];
  }
  HAMode.setName("Mode");
  HAMode.setOptions(AvailableModes.c_str());
  HAMode.onCommand(onModeCommand);
  HAMode.setIcon("mdi:stairs");

  HALDR.setName("Light");
  HALDR.setUnitOfMeasurement("lx");
  HALDR.setIcon("mdi:brightness-5");

  HALDRMax.onCommand(onNumberCommand);
  HALDRMax.setName("LDRmax");
  HALDRMax.setMin(0);
  HALDRMax.setMax(4096);
  HALDRMax.setStep(1);
  HALDRMax.setRetain(true);

  mqtt.begin(HA_BROKER_ADDR, HA_BROKER_USERNAME.c_str(), HA_BROKER_PASSWORD.c_str());
  HA_MQTT_Enabled = true;                                       //Set this before HaLoop to avoid looping
}
void HaLoop() {
  HaSetup();                                                    //Run setup if we haven't yet
  WiFiManager.RunServer();
  if (!HA_MQTT_Enabled) return;
  static unsigned long LastTime;
  if (TickEveryXms(&LastTime, HA_EveryXmsReconnect))
    WiFiManager.CheckAndReconnectIfNeeded(false);               //Try to connect to WiFi, but dont start ApMode
  mqtt.loop();
  HAMode.setState(Mode);
  static unsigned long LastTime2 = 0;
  if (TickEveryXms(&LastTime2, HA_EveryXmsUpdate)) {
    HALDR.setValue(ReadLDR());
  }
}
