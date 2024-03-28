void handle_Info() {
  String Message = "https://github.com/jellewie \n"
                   "Code compiled on " + String(__DATE__) + " " + String(__TIME__) + "\n"
                   "MAC adress = " + String(WiFi.macAddress()) + "\n"
                   "IP adress = " + IpAddress2String(WiFi.localIP()) + "\n"
                   "PAI_LDR raw= " + String(analogRead(PAI_LDR)) + "(not inversed/scaled!)\n"
                   "ReadLDR = " + String(ReadLDR()) + " dark=0 and bright=255\n"
                   "LDRmax = " + String(LDRmax) + "\n"
                   "ToBright = " + (digitalRead(ToBright) ? "FALSE" : "TRUE")  + "\n"
                   "AverageAmount = " + String(AverageAmount) + "\n"
                   "HA_EveryXmsReconnect = " + String(HA_EveryXmsReconnect) + "ms = " + String(HA_EveryXmsReconnect / 60000) +  "m\n"
                   "HA_EveryXmsUpdate = " + String(HA_EveryXmsUpdate) + "ms = " + String(HA_EveryXmsUpdate / 60000) + "m\n"
                   "LEDSections/steps = " + String(LEDSections) + "\n"
                   "Direction = " + String(Direction) + "\n"
                   "lastStep = " + String(lastStep) + "\n"

                   "\nSOFT_SETTINGS\n";
  for (byte i = 0; i < WiFiManager_Settings - 2; i++)
    Message += WiFiManager_VariableNames[i + 2] + " = " + WiFiManagerUser_Get_Value(i, false, true) + "\n";

  Message += "\nSOFT_SETTINGS raw\n";
  for (byte i = 0; i < WiFiManager_Settings - 2; i++)
    Message += WiFiManager_VariableNames[i + 2] + " = " + WiFiManagerUser_Get_Value(i, false, false) + "\n";

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
