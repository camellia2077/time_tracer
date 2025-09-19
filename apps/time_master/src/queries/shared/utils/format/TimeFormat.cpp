// queries/shared/utils/format/TimeFormat.cpp
#include "TimeFormat.hpp"
#include <sstream>
#include <iomanip>
#include <chrono>
#include <ctime>

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

std::string add_days_to_date_str(std::string date_str, int days) {
    int year = std::stoi(date_str.substr(0, 4));
    int month = std::stoi(date_str.substr(4, 2));
    int day = std::stoi(date_str.substr(6, 2));

    std::tm t{};
    t.tm_year = year - 1900;
    t.tm_mon = month - 1;
    t.tm_mday = day + days;
    std::mktime(&t);

    std::stringstream ss;
    ss << std::put_time(&t, "%Y%m%d");
    return ss.str();
}

std::string get_current_date_str() {
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&in_time_t), "%Y%m%d");
    return ss.str();
}