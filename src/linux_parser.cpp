#include <dirent.h>
#include <unistd.h>
#include <sstream>
#include <string>
#include <vector>

#include "linux_parser.h"

using std::stof;
using std::string;
using std::to_string;
using std::vector;

// DONE: An example of how to read data from the filesystem
string LinuxParser::OperatingSystem() {
  string line;
  string key;
  string value;
  std::ifstream filestream(kOSPath);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ' ', '_');
      std::replace(line.begin(), line.end(), '=', ' ');
      std::replace(line.begin(), line.end(), '"', ' ');
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "PRETTY_NAME") {
          std::replace(value.begin(), value.end(), '_', ' ');
          return value;
        }
      }
    }
  }
  filestream.close();
  return value;
}

// DONE: An example of how to read data from the filesystem
string LinuxParser::Kernel() {
  string os, kernel, version;
  string line;
  std::ifstream stream(kProcDirectory + kVersionFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> os >> version >> kernel;
  }
  stream.close();
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

float LinuxParser::MemoryUtilization() {
  string line, key, value;
  float mem_total = 0, mem_free = 0;
  std::ifstream stream(kProcDirectory + kMeminfoFilename);
  if(stream.is_open()) {
    while(std::getline(stream, line)) {
      std::remove(line.begin(), line.end(), ' ');
      std::replace(line.begin(), line.end(), ':', ' ');
      std::istringstream linestream(line);
      while(linestream >> key >> value) {
        if(key == "MemTotal") {
          mem_total = std::stof(value);
        } else if(key == "MemFree") {
          mem_free = std::stof(value);
          break;
        }
      }
    }
  }
  stream.close();
  return (mem_total - mem_free) / mem_total;
}

long LinuxParser::UpTime() {
  string line, upTime;
  std::ifstream stream(kProcDirectory + kUptimeFilename);
  if(stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> upTime;
  }
  stream.close();
  return std::stol(upTime);
}

long LinuxParser::Jiffies() {
  return LinuxParser::ActiveJiffies() + LinuxParser::IdleJiffies();
}

long LinuxParser::ActiveJiffies(int pid) {
  long totaltime;
  string line, value;
  // make sure parsing was correct and values was read
  long utime = 0, stime = 0, cutime = 0, cstime = 0;
  vector<string> values;
  std::ifstream stream(kProcDirectory + std::to_string(pid) + kStatFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    while (linestream >> value) {
      values.push_back(value);
    }

    if (std::all_of(values[13].begin(), values[13].end(), isdigit))
      utime = stol(values[13]);
    if (std::all_of(values[14].begin(), values[14].end(), isdigit))
      stime = stol(values[14]);
    if (std::all_of(values[15].begin(), values[15].end(), isdigit))
      cutime = stol(values[15]);
    if (std::all_of(values[16].begin(), values[16].end(), isdigit))
      cstime = stol(values[16]);
  }

  totaltime = utime + stime + cutime + cstime;
  return totaltime / sysconf(_SC_CLK_TCK);
}

// had to get help from this knowledge.udacity thread: https://knowledge.udacity.com/questions/900802
long LinuxParser::ActiveJiffies() {
    auto jiffies = CpuUtilization();
    return stol(jiffies[CPUStates::kUser_]) + stol(jiffies[CPUStates::kNice_]) +
          stol(jiffies[CPUStates::kSystem_]) + stol(jiffies[CPUStates::kIRQ_]) +
          stol(jiffies[CPUStates::kSoftIRQ_]) +
          stol(jiffies[CPUStates::kSteal_]);
}

// had to get help from this knowledge.udacity thread: https://knowledge.udacity.com/questions/900802
long LinuxParser::IdleJiffies() {
    auto jiffies = CpuUtilization();
    return stol(jiffies[CPUStates::kIdle_]) + stol(jiffies[CPUStates::kIOwait_]);
}

// had to get help from this knowledge.udacity thread: https://knowledge.udacity.com/questions/900802
vector<string> LinuxParser::CpuUtilization() {
  string line, value, cpu;
  vector<string> processes;
  std::ifstream stream(kProcDirectory + kStatFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> cpu;
    while (linestream >> value) {
      processes.emplace_back(value);
    }
  }
  stream.close();
  return processes;
}

// float LinuxParser::CpuUtilization(int pid) {
//   string line, value;
//   vector<string> values;
//   float utilization = 0.0;
//   long uptime = UpTime();
//   std::ifstream stream(kProcDirectory + to_string(pid) + kStatFilename);
//   if(stream.is_open()) {
//     std::getline(stream, line);
//     std::istringstream linestream(line);
//     while(linestream.good()) {
//       std::getline(linestream, value, ' ');
//       values.emplace_back(value);
//     }
//     int ticks = std::stoi(values[13]) + std::stoi(values[14]) + std::stoi(values[15]) + std::stoi(values[16]);
//     long startTime = std::stol(values[21]);
//     long totalTime = ticks / sysconf(_SC_CLK_TCK);
//     long seconds = uptime - (startTime/sysconf(_SC_CLK_TCK));
//     utilization = seconds != 0 ? (totalTime/seconds) : 0.0;
//   }
//   stream.close();
//   return utilization;
// }

int LinuxParser::TotalProcesses() {
  string key, line, value;
  int processes = 0;
  std::ifstream stream(kProcDirectory + kStatFilename);
  if(stream.is_open()) {
    while(std::getline(stream, line)) {
      std::istringstream linestream(line);
      while(linestream >> key) {
        if(key == "processes") {
          linestream >> processes;
          break;
        }
      }
    }
  }
  stream.close();
  return processes;
}

int LinuxParser::RunningProcesses() {
  string line, key;
  int processes = 0;
  std::ifstream stream(kProcDirectory + kStatFilename);
  if(stream.is_open()) {
    while(std::getline(stream, line)) {
      std::istringstream linestream(line);
      while(linestream >> key) {
        if(key == "procs_running") {
          linestream >> processes;
          break;
        }
      }
    }
  }
  stream.close();
  return processes;
}

string LinuxParser::Command(int pid) {
  string command;
  std::ifstream stream(kProcDirectory + std::to_string(pid) + kCmdlineFilename);
  if(stream.is_open()) {
    std::getline(stream, command);
  }
  stream.close();
  return command;
}

/**
 * NOTE: Suggestion in first submission review to change VmSize key to VmRSS.
 * "VmSize is the sum of all the virtual memory, whereas when you use VmRSS then it gives the exact physical 
 * memory being used as a part of Physical RAM.
 * It is recommended to replace the string VmSize with VmRSS as people who will be looking at your GitHub 
 * might not have any idea of Virtual memory and so they will think you have done something wrong."
 */
string LinuxParser::Ram(int pid) {
  string line, key, value;
  std::ifstream stream(kProcDirectory + std::to_string(pid) + kStatusFilename);
  if(stream.is_open()) {
    while(std::getline(stream, line)) {
      std::istringstream linestream(line);
      while(linestream >> key >> value) {
        if(key == "VmRSS:") {
          int size = std::stoi(value) / 1024;
          return to_string(size);
        }
      }
    }
  }
  stream.close();
  return std::string();
}

string LinuxParser::Uid(int pid) {
  string line, key, id;
  std::ifstream stream(kProcDirectory + std::to_string(pid) + kStatusFilename);
  if (stream.is_open()) {
    while (std::getline(stream, line)) {
      std::istringstream linestream(line);
      linestream >> key;
      if (key == "Uid:") {
        linestream >> id;
        break;
      }
    }
  }
  stream.close();
  return id;
}

string LinuxParser::User(int pid) {
  string line, key, value, temp, user;
  std::ifstream stream(kPasswordPath);
  string uid = Uid(pid);
  if(stream.is_open()) {
    while(std::getline(stream, line)) {
      std::replace(line.begin(), line.end(), ':', ' ');
      std::istringstream linestream(line);
      linestream >> key >> temp >> value;
      if(value == uid) {
        user = key;
        break;
      }
    }
  }
  stream.close();
  return user;
}

long LinuxParser::UpTime(int pid) {
  string line, value;
  vector<string> values;
  std::ifstream stream(kProcDirectory + std::to_string(pid) + kStatFilename);
  if(stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    while(linestream >> value) {
      values.emplace_back(value);
    }
  }
  // not really sure what to do here
  stream.close();
  int upTimePid = UpTime() - stol(values[21])/sysconf(_SC_CLK_TCK);
  return upTimePid;
}
