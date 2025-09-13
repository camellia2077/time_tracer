// reprocessing/converter/pipelines/InputParser.cpp
#include "InputParser.hpp"
#include <algorithm>
#include <cctype>
#include <iostream>
#include "common/AnsiColors.hpp" 

#include "reprocessing/validator/common/ValidatorUtils.hpp" 

namespace {
    std::string formatTime(const std::string& timeStrHHMM) {
        return (timeStrHHMM.length() == 4) ? timeStrHHMM.substr(0, 2) + ":" + timeStrHHMM.substr(2, 2) : timeStrHHMM;
    }
}

InputParser::InputParser(const ConverterConfig& config)
    : config_(config),
      wake_keywords_(config.getWakeKeywords().begin(), config.getWakeKeywords().end()) {}

// --- [核心修改] 调整整个解析逻辑以正确处理多个年份标记 ---
void InputParser::parse(std::istream& inputStream, std::function<void(InputData&)> onNewDay) {
    InputData currentDay;
    std::string line;
    std::string current_year_prefix = ""; // 用于存储当前区块的年份

    while (std::getline(inputStream, line)) {
        line = trim(line);
        if (line.empty()) continue;

        // 1. 检查是否为年份行
        if (isYearMarker(line)) {
            // 如果是一个新的年份标记，则更新当前的年份前缀
            current_year_prefix = line.substr(1); 
            continue; // 年份行处理完毕，继续下一行
        }
        
        // 2. 如果还没有读到任何年份，就忽略所有其他行
        if (current_year_prefix.empty()) {
            std::cerr << YELLOW_COLOR << "Warning: Skipping line '" << line << "' because a year header (e.g., y2025) has not been found yet." << RESET_COLOR << std::endl;
            continue;
        }

        // 3. 检查是否为日期行
        if (isNewDayMarker(line)) {
            // 如果当前已有正在处理的天，先将其提交
            if (!currentDay.date.empty()) {
                onNewDay(currentDay);
            }
            // 开始新的一天，并使用当前正确的年份前缀
            currentDay.clear();
            currentDay.date = current_year_prefix + line;
        } else {
            // 如果是其他行（备注或事件），则处理它
            parseLine(line, currentDay);
        }
    }
    // 提交文件中的最后一天
    if (!currentDay.date.empty()) {
        onNewDay(currentDay);
    }
}

bool InputParser::isYearMarker(const std::string& line) const {
    if (line.length() != 5 || line[0] != 'y') {
        return false;
    }
    return std::all_of(line.begin() + 1, line.end(), ::isdigit);
}

bool InputParser::isNewDayMarker(const std::string& line) const {
    return line.length() == 4 && std::all_of(line.begin(), line.end(), ::isdigit);
}

void InputParser::parseLine(const std::string& line, InputData& currentDay) const {
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
            // 正确处理分隔符长度
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