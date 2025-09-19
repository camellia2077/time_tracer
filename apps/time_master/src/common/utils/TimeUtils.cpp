// common/utils/TimeUtils.cpp
#include "TimeUtils.hpp"
#include <sstream>
#include <stdexcept>
#include <cctype>

int time_str_to_seconds(const std::string& time_str_in) {
    std::string time_str = time_str_in;
    if (time_str.length() == 4 && time_str.find(':') == std::string::npos) {
        bool all_digits = true;
        for (char c : time_str) {
            if (!std::isdigit(static_cast<unsigned char>(c))) {
                all_digits = false;
                break;
            }
        }
        if (all_digits) {
            time_str = time_str.substr(0, 2) + ":" + time_str.substr(2, 2);
        }
    }

    if (time_str.length() != 5 || time_str[2] != ':') {
        return 0;
    }
    try {
        int hours = std::stoi(time_str.substr(0, 2));
        int minutes = std::stoi(time_str.substr(3, 2));
        if (hours < 0 || hours > 23 || minutes < 0 || minutes > 59) {
            return 0;
        }
        return hours * 3600 + minutes * 60;
    } catch (const std::exception&) {
        return 0;
    }
}