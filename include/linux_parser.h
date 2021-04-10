#ifndef SYSTEM_PARSER_H
#define SYSTEM_PARSER_H

#include <fstream>
#include <regex>
#include <string>
#include <unordered_map>

using namespace std;

namespace LinuxParser {
constexpr int FOUND = 0;
typedef unsigned long lu;
typedef unsigned long long llu;

constexpr int kUTime = 14;
constexpr int kSTime = 15;
constexpr int kCUTime = 16;
constexpr int kCSTime = 17;
constexpr int kStartTime = 22;

const string fProcesses("processes");
const string fRunningProcesses("procs_running");
const string fMemTotal("MemTotal:");
const string fMemFree("MemFree:");
const string fBuffers("Buffers:");
const string fCached("Cached:");
const string fSReclaimable("SReclaimable:");
const string fShmem("Shmem:");
const string fCpu("cpu");
const string fUID("Uid:");
const string fProcMem("VmData:");

// Paths
const string kProcDirectory{"/proc/"};
const string kCmdlineFilename{"/cmdline"};
const string kCpuinfoFilename{"/cpuinfo"};
const string kStatusFilename{"/status"};
const string kStatFilename{"/stat"};
const string kUptimeFilename{"/uptime"};
const string kMeminfoFilename{"/meminfo"};
const string kVersionFilename{"/version"};
const string kOSPath{"/etc/os-release"};
const string kPasswordPath{"/etc/passwd"};

// System
float MemoryUtilization();
long UpTime();
vector<int> Pids();
int TotalProcesses();
int RunningProcesses();
string OperatingSystem();
string Kernel();

// CPU
const int N_STATES = 10;  // Number of elements in CPUStates
enum CPUStates {
  kUser_ = 0,
  kNice_,
  kSystem_,
  kIdle_,
  kIOwait_,
  kIRQ_,
  kSoftIRQ_,
  kSteal_,
  kGuest_,
  kGuestNice_
};
vector<string> CpuUtilization();
long Jiffies();
long ActiveJiffies();
long ActiveJiffies(int pid);
long IdleJiffies();

// Processes
string Command(int pid);
string Ram(int pid);
string Uid(int pid);
string User(int pid);
long int UpTime(int pid);

// helper functions
void InitProc();
void UpdateMeminfo();
void UpdateStat();
void UpdateUptime();

template <typename T>
T findValueByKey(string const &keyFilter, string const &filename) {
  string line, key;
  T value;

  ifstream file(kProcDirectory + filename);
  if (file.is_open()) {
    while (getline(file, line)) {
      istringstream linestream(line);
      while (linestream >> key >> value)
        if (key == keyFilter) return value;
    }
  }
  return value;
};

template <typename T>
T getValueOfFile(string const &filename) {
  string line;
  T value;

  ifstream stream(kProcDirectory + filename);
  if (stream.is_open()) {
    getline(stream, line);
    istringstream linestream(line);
    linestream >> value;
  }
  return value;
};

template <typename T>
T findNthValue(size_t const &n, string const &filename) {
  string line, temp;
  T value;

  ifstream file(kProcDirectory + filename);
  if (file.is_open()) {
    while (getline(file, line)) {
      istringstream linestream(line);
      for (size_t i = 1; i < n; i++) linestream >> temp;
      linestream >> value;
      return value;
    }
  }
  return value;
};

};  // namespace LinuxParser

#endif
