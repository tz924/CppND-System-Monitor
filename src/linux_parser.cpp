#include "linux_parser.h"

#include <dirent.h>
#include <unistd.h>

#include <cmath>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

using std::getline;
using std::ifstream;
using std::istringstream;
using std::replace;
using std::stof;
using std::string;
using std::to_string;
using std::unordered_map;
using std::vector;

// DONE: An example of how to read data from the filesystem
string LinuxParser::OperatingSystem() {
  string line;
  string key;
  string value;
  ifstream filestream(kOSPath);
  if (filestream.is_open()) {
    while (getline(filestream, line)) {
      replace(line.begin(), line.end(), ' ', '_');
      replace(line.begin(), line.end(), '=', ' ');
      replace(line.begin(), line.end(), '"', ' ');
      istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "PRETTY_NAME") {
          replace(value.begin(), value.end(), '_', ' ');
          return value;
        }
      }
    }
  }
  return value;
}

// DONE: An example of how to read data from the filesystem
string LinuxParser::Kernel() {
  string os, version, kernel;
  string line;
  ifstream stream(kProcDirectory + kVersionFilename);
  if (stream.is_open()) {
    getline(stream, line);
    istringstream linestream(line);
    linestream >> os >> version >> kernel;
  }
  return kernel;
}

// BONUS: Update this to use std::filesystem
vector<int> LinuxParser::Pids() {
  vector<int> pids;
  DIR* directory = opendir(kProcDirectory.c_str());
  struct dirent* file;
  while ((file = readdir(directory)) != nullptr) {
    // Is this a directory?
    if (file->d_type == DT_DIR) {
      // Is every character of the name a digit?
      string filename(file->d_name);
      if (std::all_of(filename.begin(), filename.end(), isdigit)) {
        int pid = stoi(filename);
        pids.push_back(pid);
      }
    }
  }
  closedir(directory);
  return pids;
}

// DONE: Read and return the system memory utilization
// source:
// https://stackoverflow.com/questions/41224738/how-to-calculate-system-memory-usage-from-proc_dict-meminfo-like-htop/41251290#41251290
float LinuxParser::MemoryUtilization() {
  InitProc();
  UpdateMeminfo();
  uint64_t memTotal{proc["meminfo"]["MemTotal"]},
      memFree{proc["meminfo"]["MemFree"]}, buffers{proc["meminfo"]["Buffers"]},
      cached{proc["meminfo"]["Cached"]},
      sReclaimable{proc["meminfo"]["SReclaimable"]},
      shmem{proc["meminfo"]["Shmem"]}, usedTotal{memTotal - memFree},
      cachedMemory{cached + sReclaimable - shmem},
      nonCachedTotal{usedTotal - (buffers + cachedMemory)};

  return (nonCachedTotal + 0.) / memTotal;
}

// DONE: Read and return the system uptime
long LinuxParser::UpTime() {
  InitProc();
  UpdateUptime();
  return proc["uptime"]["upTime"];
}

// TODO: Read and return the number of jiffies for the system
long LinuxParser::Jiffies() { return 0; }

// TODO: Read and return the number of active jiffies for a PID
// REMOVE: [[maybe_unused]] once you define the function
long LinuxParser::ActiveJiffies(int pid [[maybe_unused]]) { return 0; }

// TODO: Read and return the number of active jiffies for the system
long LinuxParser::ActiveJiffies() { return 0; }

// TODO: Read and return the number of idle jiffies for the system
long LinuxParser::IdleJiffies() { return 0; }

// TODO: Read and return CPU utilization
vector<string> LinuxParser::CpuUtilization() {
  InitProc();
  UpdateStat();

  prevIdle = previdle + previowait;
  Idle = idle + iowait;

  PrevNonIdle =
      prevuser + prevnice + prevsystem + previrq + prevsoftirq + prevsteal;
  NonIdle = user + nice + system + irq + softirq + steal;

  PrevTotal = prevIdle + PrevNonIdle;
  Total = Idle + NonIdle;

  // differentiate: actual value minus the previous one
  totald = Total - PrevTotal;
  idled = Idle - prevIdle;

  CPU_Percentage = (totald - idled) / totald;

  return {};
}

// DONE: Read and return the total number of processes
int LinuxParser::TotalProcesses() {
  InitProc();
  UpdateStat();
  return proc["stat"]["processes"];
}

// DONE: Read and return the number of running processes
int LinuxParser::RunningProcesses() {
  InitProc();
  UpdateStat();
  return proc["stat"]["procs_running"];
}

// TODO: Read and return the command associated with a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Command(int pid [[maybe_unused]]) { return string(); }

// TODO: Read and return the memory used by a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Ram(int pid [[maybe_unused]]) { return string(); }

// TODO: Read and return the user ID associated with a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Uid(int pid [[maybe_unused]]) { return string(); }

// TODO: Read and return the user associated with a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::User(int pid [[maybe_unused]]) { return string(); }

// TODO: Read and return the uptime of a process
// REMOVE: [[maybe_unused]] once you define the function
long LinuxParser::UpTime(int pid [[maybe_unused]]) { return 0; }

/****************************** helper functions ******************************/
void LinuxParser::UpdateMeminfo() {
  string line, key, unit;
  uint64_t value;

  // handles meminfo
  ifstream filestream(kProcDirectory + kMeminfoFilename);
  if (filestream.is_open()) {
    // read key value pairs into dict
    while (getline(filestream, line)) {
      istringstream linestream(line);
      while (linestream >> key >> value >> unit) {
        // remove :
        key.pop_back();
        proc["meminfo"][key] = value;
      }
    }
  }
}

void LinuxParser::UpdateStat() {
  string line, key;
  uint64_t value;

  ifstream filestream(kProcDirectory + kStatFilename);
  while (getline(filestream, line)) {
    istringstream linestream(line);
    linestream >> key;
    // for processes related
    if (key.rfind("proc", 0) == 0) {
      linestream >> value;
      proc["stat"][key] = value;
    }
    // for cpu utilization
    if (key == "cpu") {
      linestream >> value;
      proc["stat"]["user"] = value;

      linestream >> value;
      proc["stat"]["nice"] = value;

      linestream >> value;
      proc["stat"]["system"] = value;

      linestream >> value;
      proc["stat"]["idle"] = value;

      linestream >> value;
      proc["stat"]["iowait"] = value;

      linestream >> value;
      proc["stat"]["irq"] = value;

      linestream >> value;
      proc["stat"]["softirq"] = value;

      linestream >> value;
      proc["stat"]["steal"] = value;

      linestream >> value;
      proc["stat"]["guest"] = value;

      linestream >> value;
      proc["stat"]["guest_nice"] = value;
    }
  }
}

void LinuxParser::UpdateUptime() {
  string line, key;
  double upTime{0}, idle{0};

  ifstream filestream(kProcDirectory + kUptimeFilename);
  // get uptime in seconds from file (first value)
  getline(filestream, line);
  istringstream linestream(line);
  linestream >> upTime >> idle;

  // std::cout << upTime << " " << idle << '\n';

  proc["uptime"]["upTime"] = round(upTime);
  proc["uptime"]["idle"] = round(idle);
}

// TODO: inialize map if not already initialized
void LinuxParser::InitProc() {
  // handles meminfo
  if (proc.find("meminfo") == proc.end()) {
    unordered_map<string, uint64_t> meminfo;
    proc["meminfo"] = meminfo;
  }

  // handles stat
  if (proc.find("stat") == proc.end()) {
    unordered_map<string, uint64_t> stat;
    proc["stat"] = stat;
  }

  // handles uptime
  if (proc.find("uptime") == proc.end()) {
    unordered_map<string, uint64_t> uptime;
    proc["uptime"] = uptime;
  }
}
