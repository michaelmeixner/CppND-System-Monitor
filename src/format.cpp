#include <string>

#include "format.h"

using std::string;
using std::to_string;

// TODO: Complete this helper function
// INPUT: Long int measuring seconds
// OUTPUT: HH:MM:SS
// REMOVE: [[maybe_unused]] once you define the function
string Format::ElapsedTime(long int seconds) { 
    int minutes, hours;
    string hourStr, minuteStr, secondStr;
    hours = seconds / 3600;
    seconds = seconds % 3600;
    minutes = seconds / 60;
    seconds = seconds % 60;
    if(hours < 10) {
        hourStr = to_string(0) + to_string(hours);
    } else {
        hourStr = to_string(hours);
    }
    if(minutes < 10) {
        minuteStr = to_string(0) + to_string(minutes);
    } else {
        minuteStr = to_string(minutes);
    }
    if(seconds < 10) {
        secondStr = to_string(0) + to_string(seconds);
    } else {
        secondStr = to_string(seconds);
    }
    return hourStr + ":" + minuteStr + ":" + secondStr;
}