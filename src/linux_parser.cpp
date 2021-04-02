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
  LoadProc();

  uint64_t memTotal{proc["meminfo"]["MemTotal"]},
      memFree{proc["meminfo"]["MemFree"]};

  // usedTotal = memTotal - memFree;
  return (memTotal - memFree + 0.) / memTotal;
}

// DONE: Read and return the system uptime
long LinuxParser::UpTime() {
  long upTime{0};

  ifstream filestream(kProcDirectory + kUptimeFilename);
  if (filestream.is_open()) {
    double seconds;

    // get uptime in seconds from file (first value)
    string line;
    getline(filestream, line);
    istringstream linestream(line);
    linestream >> seconds;

    upTime = round(seconds);
  }

  return upTime;
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
vector<string> LinuxParser::CpuUtilization() { return {}; }

// DONE: Read and return the total number of processes
int LinuxParser::TotalProcesses() {
  LoadProc();
  return proc["stat"]["processes"];
}

// DONE: Read and return the number of running processes
int LinuxParser::RunningProcesses() {
  LoadProc();
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

// helper functions

// TODO: load meminfo into our map if not already loaded
void LinuxParser::LoadProc() {
  if (proc.empty()) {
    string line, key, unit;
    uint64_t value;

    // handles meminfo
    ifstream filestream(kProcDirectory + kMeminfoFilename);
    if (filestream.is_open()) {
      unordered_map<string, uint64_t> meminfo;

      // read key value pairs into dict
      while (getline(filestream, line)) {
        istringstream linestream(line);
        while (linestream >> key >> value >> unit) {
          // remove :
          key.pop_back();
          meminfo[key] = value;
        }
      }

      proc["meminfo"] = meminfo;
    }
    filestream.close();

    // FIXME: not stat loading correctly
    // handles stat
    filestream.open(kProcDirectory + kStatFilename);
    if (filestream.is_open()) {
      unordered_map<string, uint64_t> stat;
      // read key value pairs into dict
      while (getline(filestream, line)) {
        istringstream linestream(line);
        linestream >> key;
        if (key.rfind("proc", 0) == 0) {
          linestream >> value;
          stat[key] = value;
        }
      }
      proc["stat"] = stat;
    }
  }
};
