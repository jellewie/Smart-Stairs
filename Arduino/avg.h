#ifndef AverageAmount
# define AverageAmount 16                               //On how much points to take the average from
#else
# if AverageAmount > 64
#  Pragma "AverageAmount above 64 will overflow 'int PointTotal'"
# endif //AverageAmount > 64
#endif //AverageAmount

struct AVG {
  byte Counter;
  long PointTotal;
  byte Point[AverageAmount];
};
byte ReadAverage(byte Input, AVG *av) {
  //Returns the average of the last AverageAmount measurements
  av->PointTotal -= av->Point[av->Counter];                      //Remove the old value from the total value
  av->Point[av->Counter] = Input;
  av->PointTotal += av->Point[av->Counter];                      //Add the new value to the total value
  av->Counter++;
  if (av->Counter >= AverageAmount)
    av->Counter = 0;
  byte ReturnValue = av->PointTotal / AverageAmount;
  return ReturnValue;
}
