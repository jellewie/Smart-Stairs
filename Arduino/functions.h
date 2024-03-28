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
