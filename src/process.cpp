#include "process.h"

#include <unistd.h>

#include <cctype>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

Process::Process(int pid) : pid_{pid} {}

// DONE: Return this process's ID
int Process::Pid() { return pid_; }

// DONE: Return this process's CPU utilization
float Process::CpuUtilization() {
  long totalTime{LinuxParser::ActiveJiffies(pid_)}, upProcess{UpTime()},
      upCPU{LinuxParser::UpTime()}, Hertz{sysconf(_SC_CLK_TCK)};
  long seconds{upCPU - upProcess};

  return 1.f * totalTime / Hertz / seconds;
}

// DONE: Return the command that generated this process
string Process::Command() { return LinuxParser::Command(pid_); }

// DONE: Return this process's memory utilization
string Process::Ram() { return LinuxParser::Ram(pid_); }

// DONE: Return the user (name) that generated this process
string Process::User() { return LinuxParser::User(pid_); }

// DONE: Return the age of this process (in seconds)
long int Process::UpTime() { return LinuxParser::UpTime(pid_); }

// DONE: Overload the "less than" comparison operator for Process objects
bool Process::operator<(Process const& a) const { return pid_ < a.pid_; }
