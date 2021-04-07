#include "linux_parser.h"

#include <dirent.h>
#include <unistd.h>

#include <cmath>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

using std::any_cast;
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
  UpdateMeminfo();
  any memTotal{proc["meminfo"]["MemTotal"]},
      memFree{proc["meminfo"]["MemFree"]}, buffers{proc["meminfo"]["Buffers"]},
      cached{proc["meminfo"]["Cached"]},
      sReclaimable{proc["meminfo"]["SReclaimable"]},
      shmem{proc["meminfo"]["Shmem"]},
      usedTotal{any_cast<u64>(memTotal) - any_cast<u64>(memFree)},
      cachedMemory{any_cast<u64>(cached) + any_cast<u64>(sReclaimable) -
                   any_cast<u64>(shmem)},
      nonCachedTotal{any_cast<u64>(usedTotal) -
                     (any_cast<u64>(buffers) + any_cast<u64>(cachedMemory))};

  return (any_cast<u64>(nonCachedTotal) + 0.) / any_cast<u64>(memTotal);
}

// DONE: Read and return the system uptime
long LinuxParser::UpTime() {
  UpdateUptime();
  return any_cast<long>(proc["uptime"]["upTime"]);
}

// DONE: Read and return the number of jiffies for the system
long LinuxParser::Jiffies() { return ActiveJiffies() + IdleJiffies(); }

// TODO: Read and return the number of active jiffies for a PID
long LinuxParser::ActiveJiffies(int pid) {
  string line, temp;
  u64 utime, stime, cutime, cstime;

  ifstream filestream(kProcDirectory + to_string(pid) + "/" + kStatusFilename);
  if (filestream.is_open()) {
    while (getline(filestream, line)) {
      istringstream linestream(line);
      for (int i = 0; i < 22; i++) {
        switch (i) {
          case 13:
            linestream >> utime;
            break;
          case 14:
            linestream >> stime;
            break;
          case 15:
            linestream >> cutime;
            break;
          case 16:
            linestream >> cstime;
            break;
          default:
            linestream >> temp;
            break;
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
  UpdateStat();
  vector<string> cpu;
  string state;

  // convert cpu data to vector
  istringstream linestream(any_cast<string>(proc["stat"]["cpu"]));
  for (int i = 0; i <= N_STATES; i++) {
    // skip header "cpu"
    linestream >> state;
    if (i > 0) cpu.push_back(state);
  }

  return cpu;
}

// DONE: Read and return the total number of processes
int LinuxParser::TotalProcesses() {
  UpdateStat();
  return any_cast<int>(proc["stat"]["processes"]);
}

// DONE: Read and return the number of running processes
int LinuxParser::RunningProcesses() {
  UpdateStat();
  return any_cast<int>(proc["stat"]["procs_running"]);
}

// TODO: Read and return the command associated with a process
string LinuxParser::Command(int pid) {
  string cmdline;

  // handles meminfo
  ifstream filestream(kProcDirectory + to_string(pid) + "/" + kCmdlineFilename);
  if (filestream.is_open()) getline(filestream, cmdline);

  return cmdline;
}

// TODO: Read and return the memory used by a process
string LinuxParser::Ram(int pid) {
  string line, key;
  u64 value;

  // handles meminfo
  ifstream filestream(kProcDirectory + to_string(pid) + "/" + kStatusFilename);
  if (filestream.is_open()) {
    while (getline(filestream, line)) {
      replace(line.begin(), line.end(), ':', ' ');
      istringstream linestream(line);
      linestream >> key;
      if (key == "VmSize") linestream >> value;
    }
  }

  return to_string(static_cast<u64>(round(value / 1000.)));
}

// FIXME: Read and return the user ID associated with a process
string LinuxParser::Uid(int pid) {
  string line, key, uid;

  // handles meminfo
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

// TODO: Read and return the user associated with a process
string LinuxParser::User(int pid) {
  string line, key, uid{Uid(pid)}, user{"N/A"}, temp;
  // handles meminfo
  ifstream filestream(kPasswordPath);
  if (filestream.is_open()) {
    while (getline(filestream, line)) {
      replace(line.begin(), line.end(), ':', ' ');
      istringstream linestream(line);
      for (int i = 0; i < 3; i++) {
        if (i == 0) linestream >> user;
        if (i == 2) linestream >> key;
        linestream >> temp;
      }
      if (key == uid) return user;
    }
  }
  return user;
}

// TODO: Read and return the uptime of a process
long LinuxParser::UpTime(int pid) {
  string line, temp;
  u64 starttime;
  // handles meminfo
  ifstream filestream(kProcDirectory + to_string(pid) + "/" + kStatFilename);
  if (filestream.is_open()) {
    while (getline(filestream, line)) {
      istringstream linestream(line);
      for (size_t i = 0; i < 21; i++) linestream >> temp;
      linestream >> starttime;
    }
  }

  return round(starttime * 1. / sysconf(_SC_CLK_TCK));
}

/****************************** helper functions
 * ******************************/
void LinuxParser::UpdateMeminfo() {
  string line, key, unit;
  u64 value;
  InitProc();

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
  int value;
  InitProc();

  ifstream filestream(kProcDirectory + kStatFilename);
  while (getline(filestream, line)) {
    istringstream linestream(line);
    linestream >> key;
    // for processes related
    if (key.rfind("proc", 0) == FOUND) {
      linestream >> value;
      proc["stat"][key] = value;
    }
    // for cpu utilization
    if (key == "cpu") proc["stat"]["cpu"] = linestream.str();
  }
}

void LinuxParser::UpdateUptime() {
  string line, key;
  double upTime{0};
  InitProc();

  ifstream filestream(kProcDirectory + kUptimeFilename);
  // get uptime in seconds from file (first value)
  getline(filestream, line);
  istringstream linestream(line);
  linestream >> upTime;

  proc["uptime"]["upTime"] = static_cast<long>(round(upTime));
}

// DONE: inialize map if not already initialized
void LinuxParser::InitProc() {
  // handles meminfo
  if (proc.find("meminfo") == proc.end()) {
    unordered_map<string, any> meminfo;
    proc["meminfo"] = meminfo;
  }

  // handles stat
  if (proc.find("stat") == proc.end()) {
    unordered_map<string, any> stat;
    proc["stat"] = stat;
  }

  // handles uptime
  if (proc.find("uptime") == proc.end()) {
    unordered_map<string, any> uptime;
    proc["uptime"] = uptime;
  }
}
