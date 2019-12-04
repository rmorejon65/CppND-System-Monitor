#include <dirent.h>
#include <unistd.h>
#include <string>
#include <vector>
#include <unistd.h>
#include <sys/time.h>
#include <experimental/filesystem>
#include "linux_parser.h"

using std::stof;
using std::string;
using std::to_string;
using std::vector;

namespace fs = std::experimental::filesystem;

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
  return value;
}

// DONE: An example of how to read data from the filesystem
string LinuxParser::Kernel() {
  string os, kernel;
  string line;
  string version;
  std::ifstream stream(kProcDirectory + kVersionFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> os >> version >> kernel;
  }
  return kernel;
}

// BONUS: Update this to use std::filesystem
//vector<int> LinuxParser::Pids() {
 // vector<int> pids;
 // DIR* directory = opendir(kProcDirectory.c_str());
 // struct dirent* file;
  //while ((file = readdir(directory)) != nullptr) {
    // Is this a directory?
  //  if (file->d_type == DT_DIR) {
      // Is every character of the name a digit?
   //   string filename(file->d_name);
     // if (std::all_of(filename.begin(), filename.end(), isdigit)) {
       // int pid = stoi(filename);
        //pids.push_back(pid);
      //}
    //}
  //}
  //closedir(directory);
  //return pids;
//}

vector<int> LinuxParser::Pids() {
  vector<int> pids;
  for(const auto &entry:fs::directory_iterator(kProcDirectory.c_str())) {
    auto filename = entry.path().filename().generic_string();
    if (fs::is_directory(entry.status())) {
      if (std::all_of(filename.begin(), filename.end(), isdigit)) {
        int pid = std::stoi(filename);
        pids.push_back(pid);
      }
    }
  }
  return pids;
}
// TODO: Read and return the system memory utilization
float LinuxParser::MemoryUtilization() { 
  string line;
  string key;
  long memValue;
  long memTotal = 0;
  long memFree = 0;
  long memAvailable = 0;
  long memBuffers;
  string unit;
  std::ifstream filestream(kProcDirectory + kMeminfoFilename);

  if (filestream.is_open()) {
      int countToRead = 0;
      while (countToRead < 4 && std::getline(filestream, line)) {
          std::istringstream linestream(line);
          linestream >> key >> memValue >> unit ;
          if (key == "MemTotal:") {
            memTotal = memValue;  
            countToRead++; 
            continue;       
          }
          if (key == "MemFree:") {
            memFree = memValue;
            countToRead++;
            continue;
          }      
          if (key == "MemAvailable:") {
            memAvailable = memValue;
            countToRead++;
            continue;
          }
          if (key == "Buffers:") {
            memBuffers = memValue;
            countToRead++;
          }

      }
  }
  float memFreeGB = memFree/(1024*1024);
  float memTotalGB = memTotal/(1024*1024);
  return (memTotalGB - memFreeGB)/memTotalGB; 
  }

// TODO: Read and return the system uptime
long LinuxParser::UpTime() { 
  std::ifstream filestream(kProcDirectory + kUptimeFilename);
  long upTime = 0;
  long idleTime;
  if (filestream.is_open()) {
      std::string line;
      std::getline(filestream, line);
      std::istringstream linestream(line);
      linestream >> upTime >> idleTime;
  }
  return upTime; 
  }

// TODO: Read and return the number of jiffies for the system
long LinuxParser::Jiffies() { 
  std::ifstream filestream(kProcDirectory + kStatFilename);
  if (filestream.is_open()) {
      std::string line;
      std::getline(filestream, line);
      std::istringstream linestream(line);
      std::string cpu;
      long user;
      long nice;
      long system;
      long idle;
      long iowait;
      long irq;
      long softirq;
      long steal;
      long guess;
      long guessnice; 
      linestream >> cpu >> user >> nice >> system >> idle >> iowait >> irq >> softirq >> steal >> guess >> guessnice;
      long totalUserTime = user - guess;
      long totalNiceTime = nice - guessnice;
      long totalIdleTime = idle + iowait;
      long totalSystem = system + irq + softirq;
      long totalVirtualTime = guess + guessnice;
      return totalUserTime + totalNiceTime + totalIdleTime + totalSystem + totalVirtualTime;
  }
  return 0; }

// TODO: Read and return the number of active jiffies for a PID
// REMOVE: [[maybe_unused]] once you define the function
long LinuxParser::ActiveJiffies(int pid) {
  std::stringstream filename;
  filename << kProcDirectory << "/" << pid << "/" << kStatFilename;
  std::ifstream filestream(filename.str());
  if (filestream.is_open()) {
      std::string line;
      std::getline(filestream, line);
      std::istringstream linestream(line);
      std::string ignore;
      long utime;
      long stime;
      long cutime;
      long cstime;
      long starttime;
      for(int i = 0; i < 13; i++) linestream >> ignore;
      linestream >> utime >> stime >> cutime >> cstime ;
      for(int i = 0; i < 4; i++) linestream >> ignore;
      linestream >> starttime;
      return utime + stime + cutime + cstime +starttime;
  }
  return 0; 
  }
  
  LinuxParser::CpuProcessInfo LinuxParser::GetProcessCpuInfo(int pid) {
    std::stringstream filename;
    filename << kProcDirectory << "/" << pid << "/" << kStatFilename;
    std::ifstream filestream(filename.str());
    LinuxParser::CpuProcessInfo returnInfo;
    if (filestream.is_open()) {
        std::string line;
        std::getline(filestream, line);
        std::istringstream linestream(line);
        std::string ignore;
        long utime;
        long stime;
        long cutime;
        long cstime;
        long starttime;
        for(int i = 0; i < 13; i++) linestream >> ignore;
        linestream >> utime >> stime >> cutime >> cstime ;
        for(int i = 0; i < 4; i++) linestream >> ignore;
        linestream >> starttime;
        returnInfo.seconds = LinuxParser::UpTime() - (starttime/sysconf(_SC_CLK_TCK));
        returnInfo.totalTime = (utime + stime + cutime + cstime)/sysconf(_SC_CLK_TCK);
    }   
    return returnInfo;
  }

  float LinuxParser::CpuUtilization(int pid)
  {
    //LinuxParser::CpuProcessInfo previous = LinuxParser::GetProcessCpuInfo(pid);
    //sleep(1);
    LinuxParser::CpuProcessInfo current = LinuxParser::GetProcessCpuInfo(pid);
    long secondsd = current.seconds; // - previous.seconds;
    long totald = current.totalTime;// - previous.totalTime;
    return totald*1.0/secondsd;//secondsd;
  }

// TODO: Read and return the number of active jiffies for the system
long LinuxParser::ActiveJiffies() {
  std::ifstream filestream(kProcDirectory + kUptimeFilename);
  long upTime = 0;
  long idleTime;
  if (filestream.is_open()) {
      std::string line;
      std::getline(filestream, line);
      std::istringstream linestream(line);
      linestream >> upTime >> idleTime;
  } 
  return 0; }

// TODO: Read and return the number of idle jiffies for the system
long LinuxParser::IdleJiffies() { 
  string os, kernel;
  string line;
  std::ifstream filestream(kProcDirectory + kStatFilename);
  if (filestream.is_open()) {
      std::string line;
      std::getline(filestream, line);
      std::istringstream linestream(line);
      std::string cpu;
      long user;
      long nice;
      long system;
      long idle;
      long iowait;
      long irq;
      long softirq;
      long steal;
      long guess;
      long guessnice; 
      linestream >> cpu >> user >> nice >> system >> idle >> iowait >> irq >> softirq >> steal >> guess >> guessnice;
      long totalUserTime = user - guess;
      long totalNiceTime = nice - guessnice;
      long totalIdleTime = idle + iowait;
      long totalSystem = system + irq + softirq;
      long totalVirtualTime = guess + guessnice;
      return totalIdleTime;
  }
  return 0;
 }

vector<LinuxParser::CpuKPI> LinuxParser::CpuUtilPercentage() {
  std::ifstream filestream(kProcDirectory + kStatFilename);
  vector<LinuxParser::CpuKPI> returnVector;
  if (filestream.is_open()) {
      std::string line;
      while (std::getline(filestream, line)) {
          std::istringstream linestream(line);
          std::string cpu;
          long user;
          long nice;
          long system;
          long idle;
          long iowait;
          long irq;
          long softirq;
          long steal;
          long guess;
          long guessnice; 
          linestream >> cpu >> user >> nice >> system >> idle >> iowait >> irq >> softirq >> steal >> guess >> guessnice;
          if (cpu.substr(0,3) != "cpu")
              return returnVector;
          
          long totalIdleTime = idle + iowait;
          long totalNoIdleTime = user + nice + system + irq + softirq;
          
          CpuKPI current;
          current.idleTime = totalIdleTime;
          current.totalTime = totalIdleTime + totalNoIdleTime;

          returnVector.emplace_back(current);
      }     
      return returnVector;
  }
}
// TODO: Read and return CPU utilization

vector<string> LinuxParser::CpuUtilization() { 
  std::vector<LinuxParser::CpuKPI> previousVector = LinuxParser::CpuUtilPercentage(); 
  sleep(1);
  std::vector<LinuxParser::CpuKPI> currentVector = LinuxParser::CpuUtilPercentage(); 
  vector<std::string> returnCpu;
  for(int i = 0; i < currentVector.size(); i++) {
      std::ostringstream oCpuStream;
      long totalDelta = currentVector[i].totalTime - previousVector[i].totalTime ;
      long idleDelta = currentVector[i].idleTime - previousVector[i].idleTime ;
      oCpuStream << (totalDelta - idleDelta)*1.0/totalDelta*1.0;
      returnCpu.emplace_back(oCpuStream.str());
  }
  return returnCpu;
  }

// TODO: Read and return the total number of processes
int LinuxParser::TotalProcesses() { 
  std::ifstream filestream(kProcDirectory + kStatFilename);
   long totalProcesses = 0;
  if (filestream.is_open()) {
      std::string line;
      bool processNumberFound = false;
      
      while (std::getline(filestream, line) && !processNumberFound) {
        std::istringstream linestream(line);
        std::string key;
        linestream >> key;
        if (key == "processes")
        {          
            linestream >> totalProcesses;
            processNumberFound = true;
        }
      }

  }
  return totalProcesses; 
  }

// TODO: Read and return the number of running processes
int LinuxParser::RunningProcesses() { 
  std::ifstream filestream(kProcDirectory + kStatFilename);
   long runningProcesses = 0;
  if (filestream.is_open()) {
      std::string line;
      bool processNumberFound = false;
      
      while (std::getline(filestream, line) && !processNumberFound) {
        std::istringstream linestream(line);
        std::string key;
        linestream >> key;
        if (key == "procs_running")
        {          
            linestream >> runningProcesses;
            processNumberFound = true;
        }
      }

  }
  return runningProcesses; 
   }

// TODO: Read and return the command associated with a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Command(int pid) { 
 std::stringstream filename;
  filename << kProcDirectory << "/" << pid << "/" << kCmdlineFilename;
  std::ifstream filestream(filename.str());
  std::string cmdLine ;
  if (filestream.is_open()) {
      std::getline(filestream, cmdLine);
  }
  return cmdLine;
 }

// TODO: Read and return the memory used by a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Ram(int pid) { 
  std::stringstream filename;
  filename << kProcDirectory << "/" << pid << "/" << kStatusFilename;
  std::ifstream filestream(filename.str());
  long memory ;
  std::string unit;
  if (filestream.is_open()) {
      std::string line;
      bool foundMemory = false;
      while (!foundMemory && std::getline(filestream, line)) {
        std::istringstream linestream(line);
        std::string key;
        linestream >> key;
        if (key == "VmSize:") {
          linestream >> memory >> unit;
          foundMemory = true;
        }

      }
  }
  std::ostringstream ostream;
  ostream << memory/1024 ;
  return ostream.str(); 
  }

// TODO: Read and return the user ID associated with a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Uid(int pid) { 
  std::stringstream filename;
  filename << kProcDirectory << "/" << pid << "/" << kStatusFilename;
  std::ifstream filestream(filename.str());
  string uid ;
  if (filestream.is_open()) {
      std::string line;
      bool foundUid = false;
      while (!foundUid && std::getline(filestream, line)) {
        std::istringstream linestream(line);
        std::string key;
        linestream >> key;
        if (key == "Uid:") {
          linestream >> uid;
          foundUid = true;
        }

      }
  }
  return uid; 
 }

// TODO: Read and return the user associated with a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::User(int pid) { 
  std::string uid = Uid(pid);
  std::string userName;
  std::ifstream filestream(kPasswordPath);
   long runningProcesses = 0;
  if (filestream.is_open()) {
      std::string line;
      bool uidFound = false;
      
      while (std::getline(filestream, line) && !uidFound) {
        std::replace(line.begin(), line.end(), ' ', '_');
        std::replace(line.begin(), line.end(), ':', ' ');
        std::istringstream linestream(line);
        std::string pwd;
        std::string currentUid;
        linestream >> userName >> pwd >> currentUid;
        if (currentUid == uid)
        {          
            uidFound = true;
        }
      }
  }
  return userName; 
  }

// TODO: Read and return the uptime of a process
// REMOVE: [[maybe_unused]] once you define the function
long LinuxParser::UpTime(int pid) { 
  std::stringstream filename;
  filename << kProcDirectory << "/" << pid << "/" << kStatFilename;
  std::ifstream filestream(filename.str());
  long starttime = 0;
  if (filestream.is_open()) {
      std::string line;
      std::getline(filestream, line);
      std::istringstream linestream(line);
      std::string ignore;
      for(int i = 0; i < 21; i++) linestream >> ignore;
      linestream >> starttime;
      struct timeval tv;
      gettimeofday(&tv, 0);
      std::time_t now = std::time(0);
      std::time_t elapsedTime = LinuxParser::UpTime() - (starttime/sysconf(_SC_CLK_TCK));
      return elapsedTime;
  }
  return starttime; 
  }