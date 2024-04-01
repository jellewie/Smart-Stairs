void HaSetup() {
  if (HA_BROKER_ADDR == IPAddress(0, 0, 0, 0)) return;          //Stop HA MQTT when no IP has been setup
  device.setName(Name);
  device.setSoftwareVersion(HA_deviceSoftwareVersion);
  device.setManufacturer(HA_deviceManufacturer);
  device.setModel(HA_deviceModel);
  //device.setConfigurationUrl(IpAddress2String(WiFi.localIP()).c_str());
  light.setName(HA_lightName);
  light.onStateCommand(onStateCommand);
  numbersensor.setName("Light");
  numbersensor.setUnitOfMeasurement("lx");
  numbersensor.setIcon("mdi:brightness-5");
  number.onCommand(onNumberCommand);
  number.setName("LDRmax");
  number.setMin(0);                                             //Can be float if precision is set via the constructor
  number.setMax(4096);                                          //Can be float if precision is set via the constructor
  number.setStep(1);                                            //Minimum step: 0.001f
  number.setRetain(true);
  mqtt.begin(HA_BROKER_ADDR, HA_BROKER_USERNAME.c_str(), HA_BROKER_PASSWORD.c_str());
  HA_MQTT_Enabled = true;                                       //Set this before HaLoop to avoid looping
  HaLoop();
}
void HaLoop() {
  static unsigned long LastTime;
  if (TickEveryXms(&LastTime, HA_EveryXmsReconnect)) {
    WiFiManager.CheckAndReconnectIfNeeded(false);               //Try to connect to WiFi, but dont start ApMode
  }
  static unsigned long LastTime2;
  if (TickEveryXms(&LastTime2, HA_EveryXmsUpdate)) {
    numbersensor.setValue(ReadLDR());
  }
  WiFiManager.RunServer();
  if (HA_BROKER_ADDR == IPAddress(0, 0, 0, 0)) return;          //Stop HA MQTT when no IP has been setup
  if (!HA_MQTT_Enabled) HaSetup();                              //Run setup if we haven't yet
  mqtt.loop();
}
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
  numbersensor.setValue(ReadLDR());
}
