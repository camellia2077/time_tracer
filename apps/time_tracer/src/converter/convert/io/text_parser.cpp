// converter/convert/io/text_parser.cpp
#include "text_parser.hpp"
#include <algorithm>
#include <cctype>
#include <iostream>
#include "common/ansi_colors.hpp" 
#include "common/utils/string_utils.hpp" 

namespace {
    std::string formatTime(const std::string& timeStrHHMM) {
        return (timeStrHHMM.length() == 4) ? timeStrHHMM.substr(0, 2) + ":" + timeStrHHMM.substr(2, 2) : timeStrHHMM;
    }
}

TextParser::TextParser(const ConverterConfig& config)
    : config_(config),
      // [重构] 直接使用 public 成员
      wake_keywords_(config.wake_keywords) {}

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
    // [重构] 直接访问 public 成员 config_.remark_prefix
    const std::string& remark_prefix = config_.remark_prefix;
    
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

        // [重构] wake_keywords_ 现在是 vector，使用 std::find
        bool is_wake = false;
        for(const auto& kw : wake_keywords_) {
            if (kw == desc) {
                is_wake = true;
                break;
            }
        }

        if (is_wake) {
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