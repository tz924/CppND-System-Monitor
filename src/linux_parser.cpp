#include "linux_parser.h"

#include <dirent.h>
#include <unistd.h>

#include <cmath>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

using std::fixed;
using std::getline;
using std::ifstream;
using std::istringstream;
using std::ostringstream;
using std::replace;
using std::setprecision;
using std::setw;
using std::stof;
using std::stoi;
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
  string line, key;
  u64 value;

  unordered_map<string, u64> meminfo;

  ifstream filestream(kProcDirectory + kMeminfoFilename);
  if (filestream.is_open()) {
    // read key value pairs into dict
    while (getline(filestream, line)) {
      replace(line.begin(), line.end(), ':', ' ');
      istringstream linestream(line);
      linestream >> key >> value;
      meminfo[key] = value;
    }
  }

  u64 memTotal{meminfo["MemTotal"]}, memFree{meminfo["MemFree"]},
      buffers{meminfo["Buffers"]}, cached{meminfo["Cached"]},
      sReclaimable{meminfo["SReclaimable"]}, shmem{meminfo["Shmem"]},
      usedTotal{memTotal - memFree},
      cachedMemory{cached + sReclaimable - shmem},
      nonCachedTotal{usedTotal - (buffers + cachedMemory)};

  return (nonCachedTotal + 0.) / memTotal;
}

// DONE: Read and return the system uptime
long LinuxParser::UpTime() {
  string line;
  long upTime;

  ifstream filestream(kProcDirectory + kUptimeFilename);
  if (filestream.is_open()) {
    getline(filestream, line);
    istringstream linestream(line);
    linestream >> upTime;
  }

  return upTime;
}

// DONE: Read and return the number of jiffies for the system
long LinuxParser::Jiffies() { return ActiveJiffies() + IdleJiffies(); }

// DONE: Read and return the number of active jiffies for a PID
long LinuxParser::ActiveJiffies(int pid) {
  string line, temp;
  u64 utime, stime, cutime, cstime;

  ifstream filestream(kProcDirectory + to_string(pid) + "/" + kStatFilename);
  if (filestream.is_open()) {
    while (getline(filestream, line)) {
      istringstream linestream(line);
      for (size_t i = 1; i <= 17; i++) {
        switch (i) {
          case 14:
            linestream >> utime;
            break;
          case 15:
            linestream >> stime;
            break;
          case 16:
            linestream >> cutime;
            break;
          case 17:
            linestream >> cstime;
            break;
          default:
            linestream >> temp;
        }
      }
    }
  }

  return utime + stime + cutime + cstime;
}

// DONE: Read and return the number of active jiffies for the system
long LinuxParser::ActiveJiffies() {
  auto cpuData = CpuUtilization();

  long NonIdle{
      stol(cpuData[CPUStates::kUser_]) + stol(cpuData[CPUStates::kNice_]) +
      stol(cpuData[CPUStates::kSystem_]) + stol(cpuData[CPUStates::kIRQ_]) +
      stol(cpuData[CPUStates::kSoftIRQ_]) + stol(cpuData[CPUStates::kSteal_])};

  return NonIdle;
}

// DONE: Read and return the number of idle jiffies for the system
long LinuxParser::IdleJiffies() {
  auto cpuData = CpuUtilization();

  long idle{stol(cpuData[CPUStates::kIdle_])},
      iowait{stol(cpuData[CPUStates::kIOwait_])}, Idle{idle + iowait};

  return Idle;
}

// DONE: Read and return CPU utilization
vector<string> LinuxParser::CpuUtilization() {
  vector<string> cpu;
  string key, line, state;

  ifstream filestream(kProcDirectory + kStatFilename);
  while (getline(filestream, line)) {
    istringstream linestream(line);
    linestream >> key;
    if (key == "cpu") {
      for (size_t i = 0; i < N_STATES; i++) {
        linestream >> state;
        cpu.emplace_back(state);
      }
    }
  }
  return cpu;
}

// DONE: Read and return the total number of processes
int LinuxParser::TotalProcesses() {
  string line, key;
  int processes;

  ifstream filestream(kProcDirectory + kStatFilename);
  while (getline(filestream, line)) {
    istringstream linestream(line);
    linestream >> key;
    if (key == "processes") linestream >> processes;
  }

  return processes;
}

// DONE: Read and return the number of running processes
int LinuxParser::RunningProcesses() {
  string line, key;
  int procs_running;

  ifstream filestream(kProcDirectory + kStatFilename);
  while (getline(filestream, line)) {
    istringstream linestream(line);
    linestream >> key;
    if (key == "procs_running") linestream >> procs_running;
  }

  return procs_running;
}

// DONE: Read and return the command associated with a process
string LinuxParser::Command(int pid) {
  string cmdline;

  // handles meminfo
  ifstream filestream(kProcDirectory + to_string(pid) + "/" + kCmdlineFilename);
  if (filestream.is_open()) getline(filestream, cmdline);

  return cmdline;
}

// DONE: Read and return the memory used by a process
string LinuxParser::Ram(int pid) {
  string line, key;
  u64 value;

  ifstream filestream(kProcDirectory + to_string(pid) + "/" + kStatusFilename);
  if (filestream.is_open()) {
    while (getline(filestream, line)) {
      replace(line.begin(), line.end(), ':', ' ');
      istringstream linestream(line);
      linestream >> key;
      if (key == "VmSize") linestream >> value;
    }
  }

  ostringstream os;
  os << fixed << setprecision(3) << value / 1000.;

  return os.str();
}

// DONE: Read and return the user ID associated with a process
string LinuxParser::Uid(int pid) {
  string line, key, uid;

  ifstream filestream(kProcDirectory + to_string(pid) + "/" + kStatusFilename);
  if (filestream.is_open()) {
    while (getline(filestream, line)) {
      istringstream linestream(line);
      linestream >> key;
      if (key.rfind("Uid", 0) == FOUND) linestream >> uid;
    }
  }
  return uid;
}

// DONE: Read and return the user associated with a process
string LinuxParser::User(int pid) {
  string line, key, uid{Uid(pid)}, user{"N/A"}, temp;
  ifstream filestream(kPasswordPath);
  if (filestream.is_open()) {
    while (getline(filestream, line)) {
      replace(line.begin(), line.end(), ':', ' ');
      istringstream linestream(line);
      for (size_t i = 0; i < 3; i++) {
        if (i == 0)
          linestream >> user;
        else if (i == 2)
          linestream >> key;
        else
          linestream >> temp;
      }
      if (key == uid) return user;
    }
  }
  return user;
}

// DONE: Read and return the uptime of a process
long LinuxParser::UpTime(int pid) {
  string line, temp;
  u64 starttime;

  ifstream filestream(kProcDirectory + to_string(pid) + "/" + kStatFilename);
  if (filestream.is_open()) {
    while (getline(filestream, line)) {
      istringstream linestream(line);
      for (size_t i = 1; i < 22; i++) linestream >> temp;
      linestream >> starttime;
    }
  }
  long Hertz{sysconf(_SC_CLK_TCK)};
  return round(starttime * 1. / Hertz);
}
