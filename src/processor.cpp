#include "processor.h"

// DONE: Return the aggregate CPU utilization
float Processor::Utilization() {
  long prevTotal{LinuxParser::Jiffies()};
  long prevNonIdle{LinuxParser::ActiveJiffies()};

  std::this_thread::sleep_for(1s);

  long total{LinuxParser::Jiffies()};
  long nonIdle{LinuxParser::ActiveJiffies()};
  long totald{total - prevTotal};
  long totalNonIdeld{nonIdle - prevNonIdle};

  return static_cast<float>((totalNonIdeld + 0.) / totald);
}
