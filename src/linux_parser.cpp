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

// TODO: Read and return the system memory utilization
float LinuxParser::MemoryUtilization() {
  string line, key;
  float memTotal = 0, memFree = 0;
  std::ifstream stream(kProcDirectory + kMeminfoFilename);
  if(stream.is_open()) {
    while(std::getline(stream, line)) {
      std::istringstream linestream(line);
      linestream >> key;
      if(key == "MemTotal:") {
        linestream >> memTotal;
      } else if(key == "MemAvailable:") {
        linestream >> memFree;
        break;
      }
    }
  }
  stream.close();
  return (memTotal - memFree) / memTotal;
}

// TODO: Read and return the system uptime
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

// TODO: Read and return the number of jiffies for the system
long LinuxParser::Jiffies() {
  return LinuxParser::ActiveJiffies() + LinuxParser::IdleJiffies();
}

// TODO: Read and return the number of active jiffies for a PID
long LinuxParser::ActiveJiffies(int pid) {
  string line, jiffy;
  vector<string> jiffies;
  std::ifstream stream(kProcDirectory + std::to_string(pid) + kStatFilename);
  if(stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    while(linestream >> jiffy) {
      jiffies.push_back(jiffy);
    }
  }
  stream.close();
  return jiffies.size();
}

// TODO: Read and return the number of active jiffies for the system
// had to get help from this knowledge.udacity thread: https://knowledge.udacity.com/questions/900802
long LinuxParser::ActiveJiffies() {
    auto jiffies = CpuUtilization();
    return stol(jiffies[CPUStates::kUser_]) + stol(jiffies[CPUStates::kNice_]) +
          stol(jiffies[CPUStates::kSystem_]) + stol(jiffies[CPUStates::kIRQ_]) +
          stol(jiffies[CPUStates::kSoftIRQ_]) +
          stol(jiffies[CPUStates::kSteal_]);
}

// TODO: Read and return the number of idle jiffies for the system
// had to get help from this knowledge.udacity thread: https://knowledge.udacity.com/questions/900802
long LinuxParser::IdleJiffies() {
    auto jiffies = CpuUtilization();
    return stol(jiffies[CPUStates::kIdle_]) + stol(jiffies[CPUStates::kIOwait_]);
}

// TODO: Read and return CPU utilization
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

float LinuxParser::CpuUtilization(int pid) {
  string line, value;
  vector<string> values;
  float utilization = 0.0;
  std::ifstream stream(kProcDirectory + to_string(pid) + kStatFilename);
  if(stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    while(linestream.good()) {
      std::getline(linestream, value, ' ');
      values.emplace_back(value);
    }
    int ticks = std::stoi(values[13]) + std::stoi(values[14]) + std::stoi(values[15]) + std::stoi(values[16]);
    float totalTime = ticks / (float)sysconf(_SC_CLK_TCK);
    long seconds = LinuxParser::UpTime() - LinuxParser::UpTime(pid);
    utilization = seconds != 0 ? (totalTime/(float)seconds) : 0.0;
  }
  stream.close();
  return utilization;
}

// TODO: Read and return the total number of processes
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

// TODO: Read and return the number of running processes
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

// TODO: Read and return the command associated with a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Command(int pid) {
  string command;
  std::ifstream stream(kProcDirectory + std::to_string(pid) + kCmdlineFilename);
  if(stream.is_open()) {
    std::getline(stream, command);
  }
  stream.close();
  return command;
}

// TODO: Read and return the memory used by a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Ram(int pid) {
  long memUsed;
  string line, key;
  std::ifstream stream(kProcDirectory + std::to_string(pid) + kStatFilename);
  if(stream.is_open()) {
    while(std::getline(stream, line)) {
      std::istringstream linestream(line);
      linestream >> key;
      if(key == "VmSize:") {
        linestream >> memUsed;
        break;
      }
    }
  }
  stream.close();
  return std::to_string(memUsed);
}

// TODO: Read and return the user ID associated with a process
// REMOVE: [[maybe_unused]] once you define the function
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

// TODO: Read and return the user associated with a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::User(int pid) {
  string user, line, password, id;
  std::ifstream stream(kPasswordPath);
  if(stream.is_open()) {
    while(std::getline(stream, line)) {
      std::replace(line.begin(), line.end(), ':', ' ');
      std::istringstream linestream(line);
      while(linestream >> user >> password >> id) {
        if(id == LinuxParser::Uid(pid)) {
          break;
        }
      }
    }
  }
  stream.close();
  return user;
}

// TODO: Read and return the uptime of a process
// REMOVE: [[maybe_unused]] once you define the function
long LinuxParser::UpTime(int pid) {
  string line, value;
  long uptime;
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
  uptime = stol(values[21]);
  stream.close();
  return uptime;
}
