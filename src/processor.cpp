#include "processor.h"

// TODO: Return the aggregate CPU utilization
float Processor::Utilization() {
  // long PrevTotal{LinuxParser::Jiffies()},
  // PrevIdle{LinuxParser::IdleJiffies()}; std::this_thread::sleep_for(100ms);
  long total{LinuxParser::Jiffies()}, nonIdle{LinuxParser::ActiveJiffies()};
  // totald{Total - PrevTotal}, idled{Idle - PrevIdle};

  // return (totald - idled + 0.) / totald;
  return (nonIdle + 0.) / total;
}
