// converter/internal/InputParser.cpp
#include "InputParser.hpp"
#include <algorithm>
#include <cctype>
#include <iostream>
#include <unordered_set>
#include "common/common_utils.hpp" // 假设颜色宏定义在这里

namespace {
    std::string formatTime(const std::string& timeStrHHMM) {
        return (timeStrHHMM.length() == 4) ? timeStrHHMM.substr(0, 2) + ":" + timeStrHHMM.substr(2, 2) : timeStrHHMM;
    }
}

InputParser::InputParser(const ConverterConfig& config, const std::string& year_prefix)
    : config_(config), year_prefix_(year_prefix) {}

void InputParser::parse(std::istream& inputStream, std::function<void(InputData&)> onNewDay) {
    InputData currentDay;
    std::string line;

    while (std::getline(inputStream, line)) {
        line.erase(0, line.find_first_not_of(" \t\n\r\f\v"));
        line.erase(line.find_last_not_of(" \t\n\r\f\v") + 1);
        if (line.empty()) continue;

        if (isNewDayMarker(line)) {
            if (!currentDay.date.empty()) {
                onNewDay(currentDay);
            }
            currentDay.clear();
            currentDay.date = year_prefix_ + line;
        } else {
            parseLine(line, currentDay);
        }
    }
    if (!currentDay.date.empty()) {
        onNewDay(currentDay);
    }
}

bool InputParser::isNewDayMarker(const std::string& line) const {
    return line.length() == 4 && std::all_of(line.begin(), line.end(), ::isdigit);
}

void InputParser::parseLine(const std::string& line, InputData& currentDay) const {
    const std::string& remark_prefix = config_.getRemarkPrefix();
    const auto& wake_keywords_vec = config_.getWakeKeywords();
    const std::unordered_set<std::string> wake_keywords(wake_keywords_vec.begin(), wake_keywords_vec.end());

    if (!remark_prefix.empty() && line.rfind(remark_prefix, 0) == 0) {
        if (!currentDay.date.empty()) {
             currentDay.generalRemarks.push_back(line.substr(remark_prefix.length()));
        }
    } else if (!currentDay.date.empty() && line.length() >= 5 && std::all_of(line.begin(), line.begin() + 4, ::isdigit)) {
        std::string timeStr = line.substr(0, 4);
        std::string desc = line.substr(4);
        
        if (wake_keywords.count(desc)) {
            if (currentDay.getupTime.empty()) currentDay.getupTime = formatTime(timeStr);
        } else {
            if (currentDay.getupTime.empty() && currentDay.rawEvents.empty()) currentDay.isContinuation = true;
        }
        currentDay.rawEvents.push_back({timeStr, desc});
    } else if (!line.empty()) {
        std::cerr << YELLOW_COLOR << "Warning: Unrecognized line format for date "
                  << (currentDay.date.empty() ? "UNKNOWN" : currentDay.date)
                  << ": '" << line << "'" << RESET_COLOR << std::endl;
    }
}