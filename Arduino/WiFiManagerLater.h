/* Written by JelleWho https://github.com/jellewie
   https://github.com/jellewie/Arduino-WiFiManager
*/
//===========================================================================
// Things that can/need to be defined after including "WiFiManager.h"
//===========================================================================
const byte Pin_LED  = LED_BUILTIN;                              //Just here for some examples, It's the LED to give feedback on (like blink on error)
bool WiFiManagerUser_Set_Value(byte ValueID, String Value) {
  switch (ValueID) {                                            //Note the numbers are shifted from what is in memory, 0 is the first user value
    case 0: {
        if (Value.length() > sizeof(Name))                    return false;	//Length is to long, it would not fit so stop here
        Value.toCharArray(Name, 16);                          return true;
      } break;
    case 1: {
        if (HA_MQTT_Enabled)                                  return false; //Can only be changed before MQTT is enabled/settup
        IPAddress result = String2IpAddress(Value);
        if (result == IPAddress(0, 0, 0, 0))                  return false; //No valid IP
        HA_BROKER_ADDR           = result;                    return true;
      } break;
    case 2: {
        if (HA_MQTT_Enabled)                                  return false; //Can only be changed before MQTT is enabled/settup
        HA_BROKER_USERNAME       = Value;                     return true;
      } break;
    case 3: {
        if (HA_MQTT_Enabled)                                  return false; //Can only be changed before MQTT is enabled/settup
        HA_BROKER_PASSWORD       = Value;                     return true;
      } break;
    case 4: {
        LDRmax                   = IsTrue(Value);             return true;
      } break;
  }
  return false;                                                 //Report back that the ValueID is unknown, and we could not set it
}
String WiFiManagerUser_Get_Value(byte ValueID, bool Safe, bool Convert) {
  //if its 'Safe' to return the real value (for example the password will return '****' or '1234')
  //'Convert' the value to a readable string for the user (bool '0/1' to 'FALSE/TRUE')
  switch (ValueID) {                                            //Note the numbers are shifted from what is in memory, 0 is the first user value
    case 0:
      return String(Name);
      break;
    case 1:  return IpAddress2String(HA_BROKER_ADDR);         break;
    case 2:  return HA_BROKER_USERNAME;                       break;
    case 3:  return HA_BROKER_PASSWORD;                       break;
    case 4:  return String(LDRmax);                           break;
  }
  return "";
}
void WiFiManagerUser_Status_Start() {                           //Called before start of WiFi
  pinMode(Pin_LED, OUTPUT);
  digitalWrite(Pin_LED, HIGH);
}
void WiFiManagerUser_Status_Done() {                            //Called after succesfull connection to WiFi
  digitalWrite(Pin_LED, LOW);
}
void WiFiManagerUser_Status_Blink() {                           //Used when trying to connect/not connected
  digitalWrite(Pin_LED, !digitalRead(Pin_LED));
}
void WiFiManagerUser_Status_StartAP() {}                        //Called before start of APmode
bool WiFiManagerUser_HandleAP() {                               //Called when in the While loop in APMode, this so you can exit it
  //Return true to leave APmode
#define TimeOutApMode 15 * 60 * 1000;                           //Example for a timeout, (time in ms)
  unsigned long StopApAt = millis() + TimeOutApMode;
  if (millis() > StopApAt)    return true;                      //If we are running for to long, then flag we need to exit APMode
  return false;
}
