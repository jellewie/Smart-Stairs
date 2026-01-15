enum Modes { OFF,
             STAIRS,
             RAINBOW };                                                  //Just to make the code more clear to read, OFF=0 and ON=1 etc
String ModesString[] = { "OFF", "STAIRS", "RAINBOW" };                   //ALL CAPS!
const byte Modes_Amount = sizeof(ModesString) / sizeof(ModesString[0]);  //Why filling this in if we can automate that? :)

IPAddress String2IpAddress(String ipString) {
  IPAddress result;
  if (result.fromString(ipString))
    return result;
  return IPAddress(0, 0, 0, 0);
}
String IpAddress2String(const IPAddress& ipAddress) {
  return String(ipAddress[0]) + String(".") + String(ipAddress[1]) + String(".") + String(ipAddress[2]) + String(".") + String(ipAddress[3]);
}
bool IsTrue(String input) {
  input.toLowerCase();
  if (input == "1" or input == "true" or input == "yes" or input == "high")
    return true;
  return false;
}
bool StringIsDigit(String IN, char IgnoreCharA = '0', char IgnoreCharB = '0');
bool StringIsDigit(String IN, char IgnoreCharA, char IgnoreCharB) {
  //IgnoreChar can be used to ignore ',' or '.' or '-'
  for (byte i = 0; i < IN.length(); i++) {
    if (isDigit(IN.charAt(i))) {               //If it is a digit, do nothing
    } else if (IN.charAt(i) == IgnoreCharA) {  //If it is IgnoreCharA, do nothing
    } else if (IN.charAt(i) == IgnoreCharB) {  //If it is IgnoreCharB, do nothing
    } else {
      return false;
    }
  }
  return true;
}
String IsTrueToString(bool input) {
  if (input)
    return "true";
  return "false";
}
bool TickEveryXms(unsigned long* _LastTime, unsigned long _Delay) {
  static unsigned long _Middle = -1;
  if (_Middle == -1) _Middle = _Middle / 2;
  if (millis() - (*_LastTime + _Delay + 1) < _Middle) {
    *_LastTime = millis();
    return true;
  }
  return false;
}