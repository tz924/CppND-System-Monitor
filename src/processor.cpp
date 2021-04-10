#include "processor.h"

// DONE: Return the aggregate CPU utilization
float Processor::Utilization() {
  long prevTotal{LinuxParser::Jiffies()};
  long prevNonIdle{LinuxParser::ActiveJiffies()};

  std::this_thread::sleep_for(100ms);

  long total{LinuxParser::Jiffies()};
  long nonIdle{LinuxParser::ActiveJiffies()};
  long totald{total - prevTotal};
  float totalNonIdeld{static_cast<float>(nonIdle - prevNonIdle)};

  return totalNonIdeld / totald;
}
