// queries/shared/utils/TimeFormat.cpp
#include "TimeFormat.hpp"
#include <sstream>

std::string time_format_duration_hm(long long duration_seconds) {
    long long hours = duration_seconds / 3600;
    long long minutes = (duration_seconds % 3600) / 60;

    std::stringstream ss;
    if (hours > 0) {
        ss << hours << "h";
        if (minutes > 0) {
            ss << " ";
        }
    }
    if (minutes > 0) {
        ss << minutes << "m";
    }
    if (hours == 0 && minutes == 0) {
        ss << "0m";
    }
    return ss.str();
}