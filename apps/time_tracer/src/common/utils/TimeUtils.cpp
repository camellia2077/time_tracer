// common/utils/TimeUtils.cpp
#include "TimeUtils.hpp"
#include <sstream>
#include <stdexcept>
#include <cctype>
#include <algorithm> // for std::all_of

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

// --- [新增实现] ---

std::string normalize_to_date_format(const std::string& input) {
    // 如果是 8 位纯数字 (20250101)，转换为 2025-01-01
    if (input.length() == 8 && std::all_of(input.begin(), input.end(), ::isdigit)) {
        return input.substr(0, 4) + "-" + input.substr(4, 2) + "-" + input.substr(6, 2);
    }
    // 否则原样返回 (假设已经是 2025-01-01，或者让后续逻辑处理错误)
    return input;
}

std::string normalize_to_month_format(const std::string& input) {
    // 如果是 6 位纯数字 (202501)，转换为 2025-01
    if (input.length() == 6 && std::all_of(input.begin(), input.end(), ::isdigit)) {
        return input.substr(0, 4) + "-" + input.substr(4, 2);
    }
    return input;
}