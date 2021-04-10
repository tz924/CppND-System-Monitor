#include "linux_parser.h"

#include <dirent.h>
#include <unistd.h>

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

using namespace std;
namespace fs = filesystem;

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

// DONE: Update this to use std::filesystem
vector<int> LinuxParser::Pids() {
  vector<int> pids;

  for (auto& p : fs::directory_iterator(kProcDirectory)) {
    if (fs::is_directory(p)) {
      auto dirname = p.path().filename().string();

      // if the directory name consists of digits, add it to pids
      if (all_of(dirname.begin(), dirname.end(), ::isdigit)) {
        auto pid = stoi(dirname);
        pids.emplace_back(pid);
      }
    }
  }

  return pids;
}

// DONE: Read and return the system memory utilization
// source:
// https://stackoverflow.com/questions/41224738/how-to-calculate-system-memory-usage-from-proc_dict-meminfo-like-htop/41251290#41251290
float LinuxParser::MemoryUtilization() {
  float memTotal{findValueByKey<float>(fMemTotal, kMeminfoFilename)},
      memFree{findValueByKey<float>(fMemFree, kMeminfoFilename)},
      buffers{findValueByKey<float>(fBuffers, kMeminfoFilename)},
      cached{findValueByKey<float>(fCached, kMeminfoFilename)},
      sReclaimable{findValueByKey<float>(fSReclaimable, kMeminfoFilename)},
      shmem{findValueByKey<float>(fShmem, kMeminfoFilename)},

      usedTotal{memTotal - memFree},
      cachedMemory{cached + sReclaimable - shmem},
      nonCachedTotal{usedTotal - (buffers + cachedMemory)};

  return nonCachedTotal / memTotal;
}

// DONE: Read and return the system uptime
long LinuxParser::UpTime() { return getValueOfFile<long>(kUptimeFilename); }

// DONE: Read and return the number of jiffies for the system
long LinuxParser::Jiffies() { return ActiveJiffies() + IdleJiffies(); }

// DONE: Read and return the number of active jiffies for a PID
long LinuxParser::ActiveJiffies(int pid) {
  long utime{findNthValue<long>(kUTime, to_string(pid) + kStatFilename)},
      stime{findNthValue<long>(kSTime, to_string(pid) + kStatFilename)},
      cutime{findNthValue<long>(kCUTime, to_string(pid) + kStatFilename)},
      cstime{findNthValue<long>(kCSTime, to_string(pid) + kStatFilename)};

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
      iowait{stol(cpuData[CPUStates::kIOwait_])},

      Idle{idle + iowait};

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
    if (key == fCpu) {
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
  return findValueByKey<int>(fProcesses, kStatFilename);
}

// DONE: Read and return the number of running processes
int LinuxParser::RunningProcesses() {
  return findValueByKey<int>(fRunningProcesses, kStatFilename);
}

// DONE: Read and return the command associated with a process
string LinuxParser::Command(int pid) {
  return getValueOfFile<string>(to_string(pid) + kCmdlineFilename);
}

// DONE: Read and return the memory used by a process
// VmSize can be more than physical RAM size;
// using VmData instead to reflect accurate physical RAM usage
string LinuxParser::Ram(int pid) {
  float ram{findValueByKey<float>(fProcMem, to_string(pid) + kStatusFilename)};

  ostringstream os;
  // Convert to MB
  os << fixed << setprecision(2) << ram / 1000;

  return os.str();
}

// DONE: Read and return the user ID associated with a process
string LinuxParser::Uid(int pid) {
  return findValueByKey<string>(fUID, to_string(pid) + kStatusFilename);
}

// DONE: Read and return the user associated with a process
string LinuxParser::User(int pid) {
  string line, key, uid{Uid(pid)}, user, temp;
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
  llu start_time{findNthValue<llu>(kStartTime, to_string(pid) + kStatFilename)};
  long Hertz{sysconf(_SC_CLK_TCK)};

  return UpTime() - 1. * start_time / Hertz;
}
