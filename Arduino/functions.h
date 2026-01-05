enum Modes {OFF, STAIRS, RAINBOW};                              //Just to make the code more clear to read, OFF=0 and ON=1 etc
String ModesString[] = {"OFF", "STAIRS", "RAINBOW"};            //ALL CAPS!
const byte Modes_Amount = sizeof(ModesString) / sizeof(ModesString[0]);//Why filling this in if we can automate that? :)

int ReadAverage(int Input, AVG *av) {                           //Returns the average of the last AverageAmount measurements.
  av->PointTotal -= av->Point[av->Counter];                     //Remove the old value from the total value
  av->Point[av->Counter] = Input;                               //Add the new number into the array
  av->PointTotal += av->Point[av->Counter];                     //Add the new value to the total value
  av->Counter++;                                                //Increase the counter
  if (av->Counter >= AverageAmount)                             //If the counter is at its configured end
    av->Counter = 0;                                            //Go back down
  int ReturnValue = av->PointTotal / AverageAmount;             //Calculate the average
  return ReturnValue;
}
IPAddress String2IpAddress(String ipString) {
  IPAddress result;
  if (result.fromString(ipString))
    return result;
  return IPAddress(0, 0, 0, 0);
}
String IpAddress2String(const IPAddress& ipAddress) {
  return String(ipAddress[0]) + String(".") + String(ipAddress[1]) + String(".") + String(ipAddress[2]) + String(".") + String(ipAddress[3])  ;
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
    if (isDigit(IN.charAt(i))) {                                //If it is a digit, do nothing
    } else if (IN.charAt(i) == IgnoreCharA) {                   //If it is IgnoreCharA, do nothing
    } else if (IN.charAt(i) == IgnoreCharB) {                   //If it is IgnoreCharB, do nothing
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
bool TickEveryXms(unsigned long * _LastTime, unsigned long _Delay) {
  static unsigned long _Middle = -1;
  if (_Middle == -1) _Middle = _Middle / 2;
  if (millis() - (*_LastTime + _Delay + 1) < _Middle) {
    *_LastTime = millis();
    return true;
  }
  return false;
}
void LED_Fill(int From, int Amount, CRGB Color, int MaxBound = TotalLEDs);
void LED_Fill(int From, int Amount, CRGB Color, int MaxBound) {
  if (From < 0 || Amount <= 0 || From >= MaxBound) return;  //(Protection out of array bounds) fully out of bounds
  if (From + Amount > MaxBound) Amount = MaxBound - From;   //(Protection out of array bounds)  truncate amount if it would exceed the array
  fill_solid(&(LEDs[From]), Amount, Color);
}