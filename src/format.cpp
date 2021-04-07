#include "format.h"

#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

using std::ostringstream;
using std::setw;
using std::string;

// DONE: Complete this helper function
// INPUT: Long int measuring seconds
// OUTPUT: HH:MM:SS
string Format::ElapsedTime(long seconds) {
  long hours = seconds / 3600l;
  seconds %= 3600l;
  long minutes = seconds / 60l;
  seconds %= 60l;

  ostringstream os;
  os.fill('0');
  os << setw(2) << hours << ':' << setw(2) << minutes << ':' << setw(2)
     << seconds;

  return os.str();
}
