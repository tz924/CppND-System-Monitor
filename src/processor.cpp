#include "processor.h"

// DONE: Return the aggregate CPU utilization
float Processor::Utilization() {
  long prevTotal{LinuxParser::Jiffies()};
  long prevNonIdle{LinuxParser::ActiveJiffies()};

  std::this_thread::sleep_for(200ms);

  const long total{LinuxParser::Jiffies()};
  const long nonIdle{LinuxParser::ActiveJiffies()};
  const long totald{total - prevTotal};
  const long totalNonIdeld{nonIdle - prevNonIdle};

  return static_cast<float>(totalNonIdeld) / totald;
}
