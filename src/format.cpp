#include <string>
#include <iostream>
#include <iomanip>
#include <sstream>
#include "format.h"

using std::string;

// TODO: Complete this helper function
// INPUT: Long int measuring seconds
// OUTPUT: HH:MM:SS
// REMOVE: [[maybe_unused]] once you define the function
string Format::ElapsedTime(long seconds) { 
    std::ostringstream formatTime;
    long hours = seconds/3600;
    long rest = seconds % 3600;
    long minutes = rest/60;
    rest = rest % 60;
    formatTime << std::setfill('0') << std::setw(2) << hours  << ":"  << std::setfill('0') << std::setw(2) << minutes << ":" << std::setfill('0') << std::setw(2) << rest;
    return formatTime.str();
     }