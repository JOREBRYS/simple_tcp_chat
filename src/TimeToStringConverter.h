#ifndef TIMETOSTRINGCONVERTER_H
#define TIMETOSTRINGCONVERTER_H

#include <string>
#include <chrono>
#include <iomanip>
#include <sstream>

inline std::string ConvertTimeToString(std::chrono::system_clock::time_point time){
    auto time_t = std::chrono::system_clock::to_time_t(time);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

#endif