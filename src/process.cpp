#include "process.h"

#include <unistd.h>

#include <cctype>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

Process::Process(int pid) : pid_{pid} {}

// DONE: Return this process's ID
int Process::Pid() const { return pid_; }

// DONE: Return this process's CPU utilization
float Process::CpuUtilization() const {
  long totalTime{LinuxParser::ActiveJiffies(pid_)};
  long Hertz{sysconf(_SC_CLK_TCK)};
  long seconds{LinuxParser::UpTime() - UpTime()};
  return (1. * totalTime / Hertz) / seconds;
}

// DONE: Return the command that generated this process
string Process::Command() {
  auto command = LinuxParser::Command(pid_);

  // truncate command if it exceeds the maximum length
  return command.length() > COMMAND_MAX ? command.substr(0, COMMAND_MAX) + "..."
                                        : command;
}

// DONE: Return this process's memory utilization
string Process::Ram() { return LinuxParser::Ram(pid_); }

// DONE: Return the user (name) that generated this process
string Process::User() { return LinuxParser::User(pid_); }

// DONE: Return the age of this process (in seconds)
long int Process::UpTime() const { return LinuxParser::UpTime(pid_); }

// DONE: Overload the "less than" comparison operator for Process objects
bool Process::operator<(Process const& a) const {
  return CpuUtilization() < a.CpuUtilization();
}
// DONE: Overload the "more than" comparison operator for Process objects
bool Process::operator>(Process const& a) const {
  return CpuUtilization() > a.CpuUtilization();
}
