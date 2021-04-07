#include "process.h"

#include <unistd.h>

#include <cctype>
#include <sstream>
#include <string>
#include <vector>

using std::string;
using std::to_string;
using std::vector;

Process::Process(int pid) : pid_{pid} {}

// DONE: Return this process's ID
int Process::Pid() { return pid_; }

// TODO: Return this process's CPU utilization
float Process::CpuUtilization() {
  long totalTime{LinuxParser::ActiveJiffies(pid_)}, starttime{UpTime()},
      Hertz{sysconf(_SC_CLK_TCK)}, uptime{LinuxParser::UpTime()};
  double seconds{uptime - (starttime * 1. / Hertz)},
      cpuUsage{100. * ((totalTime * 1. / Hertz) / seconds)};

  return cpuUsage;
}

// TODO: Return the command that generated this process
string Process::Command() { return LinuxParser::Command(pid_); }

// TODO: Return this process's memory utilization
string Process::Ram() { return LinuxParser::Ram(pid_); }

// TODO: Return the user (name) that generated this process
string Process::User() { return LinuxParser::User(pid_); }

// TODO: Return the age of this process (in seconds)
long int Process::UpTime() { return LinuxParser::UpTime(pid_); }

// TODO: Overload the "less than" comparison operator for Process objects
bool Process::operator<(Process const& a) const { return pid_ < a.pid_; }
