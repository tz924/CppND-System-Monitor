#include "processor.h"

// DONE: Return the aggregate CPU utilization
float Processor::Utilization() {
  long prevTotal{LinuxParser::Jiffies()},
      prevNonIdle{LinuxParser::ActiveJiffies()};

  std::this_thread::sleep_for(1s);

  long total{LinuxParser::Jiffies()}, nonIdle{LinuxParser::ActiveJiffies()},
  totald{total - prevTotal}, totalNonIdeld{nonIdle - prevNonIdle};

  return (totalNonIdeld + 0.) / totald;
}
