#include "processor.h"
#include "linux_parser.h"

// TODO: Return the aggregate CPU utilization
// had to get help from this knowledge.udacity thread: https://knowledge.udacity.com/questions/900802
float Processor::Utilization() {
    return float(LinuxParser::ActiveJiffies())/float(LinuxParser::Jiffies());
}