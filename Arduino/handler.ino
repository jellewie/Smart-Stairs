void handle_Info() {
  String AvailableModes;
  for (size_t i = 0; i < Modes_Amount; i++) {
    if (i > 0) {
      AvailableModes += ';';
    }
    AvailableModes += ModesString[i];
  }
  String Message = "https://github.com/jellewie \n"
                   "Code compiled on " + String(__DATE__) + " " + String(__TIME__) + "\n"
                   "MAC adress = " + String(WiFi.macAddress()) + "\n"
                   "IP adress = " + IpAddress2String(WiFi.localIP()) + "\n"
                   "PAI_LDR raw= " + String(analogRead(PAI_LDR)) + " (not inversed/scaled!)\n"
                   "ReadLDR = " + String(ReadLDR()) + " dark=0 and bright=4096\n"
                   "LDRmax = " + String(LDRmax) + "\n"
                   "TooBright = " + (digitalRead(TooBright) ? "TRUE" : "FALSE")  + "\n"
                   "AverageAmount = " + String(AverageAmount) + "\n"
                   "HA_EveryXmsReconnect = " + String(HA_EveryXmsReconnect) + "ms = " + String(HA_EveryXmsReconnect / 60000) +  "m\n"
                   "HA_EveryXmsUpdate = " + String(HA_EveryXmsUpdate) + "ms = " + String(HA_EveryXmsUpdate / 60000) + "m\n"
                   "LEDSections/steps = " + String(LEDSections) + "\n"
                   "Direction = " + String(Direction) + "\n"
                   "lastStep = " + String(lastStep) + "\n"
                   "Mode  = " + String(Mode) + "\n"
                   "HA_MQTT_Enabled = " + (HA_MQTT_Enabled ? "TRUE" : "FALSE") + "\n"
                   "HA_StepDetected = " + (HA_StepDetected ? "TRUE" : "FALSE") + "\n"
                   "AvailableModes = " + String(Modes_Amount) + "x = " + AvailableModes + "\n";

  Message += "\nSteps raw\n";
  for (byte i = 0; i < LEDSections; i++)
    Message += "Step " + String(i) + " L=" + String(Stair[i].SectionLength) + " now=" + String(StepRead(i)) + " StayOnFor=" + String(Stair[i].StayOnFor) + "\n";

  Message += "\nSOFT_SETTINGS\n";
  for (byte i = 0; i < WiFiManager_Settings - 2; i++)
    Message += WiFiManager_VariableNames[i + 2] + " = " + WiFiManagerUser_Get_Value(i, false, true) + "\n";

  server.send(200, "text/plain", Message);
}
void handle_NotFound() {
  String Message = "ERROR URL NOT FOUND: '";
  Message += (server.method() == HTTP_GET) ? "GET" : "POST";
  Message += server.uri();
  if (server.args() > 0) Message += " ? ";
  for (byte i = 0; i < server.args(); i++) {
    if (i != 0)
      Message += "&";
    Message += server.argName(i) + " = " + server.arg(i);
  }
  server.send(404, "text / plain", Message);
}
