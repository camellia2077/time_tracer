// converter/convert/io/TextParser.cpp
#include "TextParser.hpp"
#include <algorithm>
#include <cctype>
#include <iostream>
#include "common/AnsiColors.hpp" 
#include "common/utils/StringUtils.hpp" 

namespace {
    std::string formatTime(const std::string& timeStrHHMM) {
        return (timeStrHHMM.length() == 4) ? timeStrHHMM.substr(0, 2) + ":" + timeStrHHMM.substr(2, 2) : timeStrHHMM;
    }
}

TextParser::TextParser(const ConverterConfig& config)
    : config_(config),
      wake_keywords_(config.getWakeKeywords().begin(), config.getWakeKeywords().end()) {}

void TextParser::parse(std::istream& inputStream, std::function<void(DailyLog&)> onNewDay) {
    DailyLog currentDay;
    std::string line;
    std::string current_year_prefix = "";

    while (std::getline(inputStream, line)) {
        line = trim(line);
        if (line.empty()) continue;

        if (isYearMarker(line)) {
            current_year_prefix = line.substr(1); 
            continue;
        }
        
        if (current_year_prefix.empty()) {
            std::cerr << YELLOW_COLOR << "Warning: Skipping line '" << line << "' because a year header (e.g., y2025) has not been found yet." << RESET_COLOR << std::endl;
            continue;
        }

        if (isNewDayMarker(line)) {
            if (!currentDay.date.empty()) {
                onNewDay(currentDay);
            }
            currentDay.clear();
            
            // [核心修改] 这里直接构造标准格式 YYYY-MM-DD
            // line 是 "0101" (MMDD)
            // 结果: "2025-01-01"
            currentDay.date = current_year_prefix + "-" + line.substr(0, 2) + "-" + line.substr(2, 2);
            
        } else {
            parseLine(line, currentDay);
        }
    }
    if (!currentDay.date.empty()) {
        onNewDay(currentDay);
    }
}

bool TextParser::isYearMarker(const std::string& line) const {
    if (line.length() != 5 || line[0] != 'y') {
        return false;
    }
    return std::all_of(line.begin() + 1, line.end(), ::isdigit);
}

bool TextParser::isNewDayMarker(const std::string& line) const {
    return line.length() == 4 && std::all_of(line.begin(), line.end(), ::isdigit);
}

void TextParser::parseLine(const std::string& line, DailyLog& currentDay) const {
    const std::string& remark_prefix = config_.getRemarkPrefix();
    if (!remark_prefix.empty() && line.rfind(remark_prefix, 0) == 0) {
        if (!currentDay.date.empty()) {
             currentDay.generalRemarks.push_back(line.substr(remark_prefix.length()));
        }
    } else if (!currentDay.date.empty() && line.length() >= 5 && std::all_of(line.begin(), line.begin() + 4, ::isdigit)) {
        std::string timeStr = line.substr(0, 4);
        std::string remaining_line = line.substr(4);
        
        std::string desc;
        std::string remark;

        size_t comment_pos = std::string::npos;
        const char* delimiters[] = {"//", "#", ";"};
        for (const char* delim : delimiters) {
            size_t pos = remaining_line.find(delim);
            if (pos != std::string::npos) {
                if (comment_pos == std::string::npos || pos < comment_pos) {
                    comment_pos = pos;
                }
            }
        }
        
        if (comment_pos != std::string::npos) {
            desc = trim(remaining_line.substr(0, comment_pos));
            size_t delim_len = (remaining_line.substr(comment_pos, 2) == "//") ? 2 : 1;
            remark = trim(remaining_line.substr(comment_pos + delim_len));
        } else {
            desc = trim(remaining_line);
        }

        if (wake_keywords_.count(desc)) {
            if (currentDay.getupTime.empty()) currentDay.getupTime = formatTime(timeStr);
        } else {
            if (currentDay.getupTime.empty() && currentDay.rawEvents.empty()) currentDay.isContinuation = true;
        }
        currentDay.rawEvents.push_back({timeStr, desc, remark});
    } else if (!line.empty()) {
        std::cerr << YELLOW_COLOR << "Warning: Unrecognized line format for date "
                  << (currentDay.date.empty() ? "UNKNOWN" : currentDay.date)
                  << ": '" << line << "'" << RESET_COLOR << std::endl;
    }
}