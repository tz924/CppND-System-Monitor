#include "format.h"

#include <chrono>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

using namespace std;

// DONE: Complete this helper function
// INPUT: Long int measuring seconds
// OUTPUT: HH:MM:SS
string Format::ElapsedTime(long secs) {
  chrono::seconds seconds{secs};

  // calculate hours and update seconds
  auto hours = chrono::duration_cast<chrono::hours>(seconds);
  seconds -= chrono::duration_cast<chrono::seconds>(hours);

  // calculate minutes and update seconds
  auto minutes = chrono::duration_cast<chrono::minutes>(seconds);
  seconds -= chrono::duration_cast<chrono::seconds>(minutes);

  // generate desired output format
  stringstream ss{};

  ss << setw(2) << setfill('0') << hours.count()     // HH
     << setw(1) << ":"                               // :
     << setw(2) << setfill('0') << minutes.count()   // MM
     << setw(1) << ":"                               // :
     << setw(2) << setfill('0') << seconds.count();  // SS

  return ss.str();
}
