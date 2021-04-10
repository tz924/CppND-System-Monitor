#ifndef PROCESSOR_H
#define PROCESSOR_H

#include <chrono>
#include <thread>

#include "linux_parser.h"
using namespace std::literals::chrono_literals;
class Processor {
 public:
  float Utilization();  // DONE: See src/processor.cpp

  // DONE: Declare any necessary private members
 private:
};

#endif
