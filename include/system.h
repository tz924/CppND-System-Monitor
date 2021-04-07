#ifndef SYSTEM_H
#define SYSTEM_H

#include <string>
#include <vector>

#include "process.h"
#include "processor.h"

using std::vector;

class System {
 public:
  // Constructor
  System();

  Processor& Cpu();                   // DONE: See src/system.cpp change
  std::vector<Process>& Processes();  // DONE: See src/system.cpp
  float MemoryUtilization();          // DONE: See src/system.cpp
  long UpTime();                      // DONE: See src/system.cpp
  int TotalProcesses();               // DONE: See src/system.cpp
  int RunningProcesses();             // DONE: See src/system.cpp
  std::string Kernel();               // DONE: See src/system.cpp
  std::string OperatingSystem();      // DONE: See src/system.cpp

  // DONE: Define any necessary private members
 private:
  Processor cpu_ {};
  vector<Process> processes_ {};
  vector<int> pids_ {};
};

#endif
