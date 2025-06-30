// --- START OF FILE reprocessing/IntervalProcessor.cpp (修复后) ---

#include "IntervalProcessor.h"

#include "common_utils.h" // 获取颜色

#include <iostream>
#include <fstream>
#include <algorithm>
#include <cctype>
#include <ctime>
#include <filesystem>

// --- 构造函数与 DayData::clear() 保持不变 ---
IntervalProcessor::IntervalProcessor(const std::string& config_filename)
    : config_filepath_(config_filename) {
    if (!loadConfiguration()) {
        std::cerr << RED_COLOR << "Error: IntervalProcessor failed to load its configuration." << RESET_COLOR << std::endl;
    }
}

void IntervalProcessor::DayData::clear() {
    date.clear();
    hasStudyActivity = false;
    endsWithSleepNight = false;
    getupTime.clear();
    generalRemarks.clear();
    rawEvents.clear();
    remarksOutput.clear();
    isContinuation = false;
}

// ##################################################################
// ###                  核心转换逻辑 (重构后)                     ###
// ##################################################################

bool IntervalProcessor::executeConversion(const std::string& input_filepath, const std::string& output_filepath, const std::string& year_prefix) {
    std::ifstream inFile(input_filepath);
    if (!inFile.is_open()) {
        std::cerr << RED_COLOR << "Error: Could not open input file " << input_filepath << RESET_COLOR << std::endl;
        return false;
    }

    std::ofstream outFile(output_filepath);
    if (!outFile.is_open()) {
        std::cerr << RED_COLOR << "Error: Could not open output file " << output_filepath << RESET_COLOR << std::endl;
        return false;
    }

    DayData previousDay, currentDay;
    std::string line;

    // 主循环: 只负责读取和分派任务
    while (std::getline(inFile, line)) {
        line.erase(0, line.find_first_not_of(" \t\n\r\f\v"));
        line.erase(line.find_last_not_of(" \t\n\r\f\v") + 1);
        if (line.empty()) continue;

        if (isDateLine(line)) {
            handleDateLine(line, currentDay, previousDay, outFile, year_prefix);
        } else if (isRemarkLine(line)) {
            handleRemarkLine(line, currentDay);
        } else { // 假定剩下的是事件行
            handleEventLine(line, currentDay);
        }
    }

    // 收尾工作: 处理并写入文件中最后两天的数据
    finalizeAndWriteDay(previousDay, currentDay, outFile);
    
    // 对于文件中的最后一天，没有 "nextDay"，所以单独处理和写入
    if (!currentDay.date.empty()) {
        processAndFormatEvents(currentDay);
        writeDayData(outFile, currentDay);
    }
    
    return true;
}


// ##################################################################
// ###                  私有辅助函数 (新实现)                     ###
// ##################################################################

void IntervalProcessor::handleDateLine(const std::string& line, DayData& currentDay, DayData& previousDay, std::ofstream& outFile, const std::string& year_prefix) {
    // 当遇到新日期时，意味着前一天的数据已经完整，可以进行最终处理和写入
    finalizeAndWriteDay(previousDay, currentDay, outFile);
    
    // “滚动”日期：当前天变为前一天，并为新的一天重置数据
    previousDay = currentDay;
    currentDay.clear();
    currentDay.date = year_prefix + line;
}

void IntervalProcessor::handleRemarkLine(const std::string& line, DayData& currentDay) {
    if (currentDay.date.empty()) return; // 忽略在第一个日期前的任何备注

    std::string remark_content = line.substr(remark_prefix_.length());
    remark_content.erase(0, remark_content.find_first_not_of(" \t"));
    if (!remark_content.empty()) {
        currentDay.generalRemarks.push_back(remark_content);
    }
}

void IntervalProcessor::handleEventLine(const std::string& line, DayData& currentDay) {
    if (currentDay.date.empty()) return; // 忽略在第一个日期前的任何事件

    std::string eventTimeBuffer, eventDescBuffer;
    if (parseEventLine(line, eventTimeBuffer, eventDescBuffer)) {
        if (eventDescBuffer == "起床" || eventDescBuffer == "醒") {
            if (currentDay.getupTime.empty()) {
                currentDay.getupTime = formatTime(eventTimeBuffer);
            }
        } else {
            // 如果在“起床”事件之前有其他事件，说明这一天是前一天的延续
            if (currentDay.getupTime.empty() && currentDay.rawEvents.empty()) {
                currentDay.isContinuation = true;
            }
        }
        currentDay.rawEvents.push_back({eventTimeBuffer, eventDescBuffer});
    }
}

void IntervalProcessor::processAndFormatEvents(DayData& day) {
    // 1. 添加通用备注
    for (const auto& general_remark : day.generalRemarks) {
         day.remarksOutput.push_back(general_remark);
    }
    
    // 如果没有起床时间且不是延续日，则没有活动事件可处理
    if (day.getupTime.empty() && !day.isContinuation) return;
    
    // 2. 遍历原始事件并格式化
    std::string startTime = day.getupTime;
    for (const auto& rawEvent : day.rawEvents) {
        std::string formattedEventEndTime = formatTime(rawEvent.endTimeStr);
        std::string mappedDescription = rawEvent.description;

        // 应用文本映射规则
        auto mapIt = text_mapping_.find(rawEvent.description);
        if (mapIt != text_mapping_.end()) {
            mappedDescription = mapIt->second;
        } else {
            auto durMapIt = text_duration_mapping_.find(rawEvent.description);
            if (durMapIt != text_duration_mapping_.end()) {
                mappedDescription = durMapIt->second;
            }
        }

        // 应用时长规则
        auto durationRulesIt = duration_mappings_.find(mappedDescription);
        if (durationRulesIt != duration_mappings_.end()) {
            int duration = calculateDurationMinutes(startTime, formattedEventEndTime);
            for (const auto& rule : durationRulesIt->second) {
                if (duration < rule.less_than_minutes) {
                    mappedDescription = rule.value;
                    break;
                }
            }
        }

        // 检查学习状态并生成输出行
        if (mappedDescription.find("study") != std::string::npos) day.hasStudyActivity = true;
        day.remarksOutput.push_back(startTime + "~" + formattedEventEndTime + mappedDescription);
        startTime = formattedEventEndTime;
    }
}

void IntervalProcessor::finalizeAndWriteDay(DayData& dayToFinalize, DayData& nextDay, std::ofstream& outFile) {
    if (dayToFinalize.date.empty()) return;

    // 1. 处理当天的所有内部事件
    processAndFormatEvents(dayToFinalize);

    // 2. 处理与第二天的关联（即 sleep_night）
    if (nextDay.isContinuation) {
        // 如果第二天是延续，那么今天就没有“夜间睡眠”
        dayToFinalize.endsWithSleepNight = false;
        if (!dayToFinalize.rawEvents.empty()) {
            // 设置第二天的“起床时间”为今天的最后一个事件的结束时间
            nextDay.getupTime = formatTime(dayToFinalize.rawEvents.back().endTimeStr);
        }
    } else if (!dayToFinalize.isContinuation && !nextDay.getupTime.empty()) {
        // 如果今天不是延续日，并且第二天有明确的起床时间，则计算夜间睡眠
        if (!dayToFinalize.rawEvents.empty()) {
            std::string sleepStartTime = formatTime(dayToFinalize.rawEvents.back().endTimeStr);
            dayToFinalize.remarksOutput.push_back(sleepStartTime + "~" + nextDay.getupTime + "sleep_night");
            dayToFinalize.endsWithSleepNight = true;
        }
    }

    // 3. 将完全处理好的一天数据写入文件
    writeDayData(outFile, dayToFinalize);
}


// ##################################################################
// ###                  底层私有函数 (无变化)                      ###
// ##################################################################

void IntervalProcessor::writeDayData(std::ofstream& outFile, const DayData& day) {
    if (day.date.empty()) return;

    for (const auto& header : header_order_) {
        if (header == "Date:") {
            outFile << "Date:" << day.date << "\n";
        } else if (header == "Status:") {
            outFile << "Status:" << (day.hasStudyActivity ? "True" : "False") << "\n";
        } else if (header == "Sleep:") {
            bool sleepStatus = day.isContinuation ? false : day.endsWithSleepNight;
            outFile << "Sleep:" << (sleepStatus ? "True" : "False") << "\n";
        } else if (header == "Getup:") {
            if (day.isContinuation) {
                outFile << "Getup:Null\n";
            } else {
                outFile << "Getup:" << (day.getupTime.empty() ? "00:00" : day.getupTime) << "\n";
            }
        } else if (header == "Remark:") {
            if (day.remarksOutput.empty()) {
                outFile << "Remark:\n";
            } else {
                outFile << "Remark:" << day.remarksOutput[0] << "\n";
                for (size_t i = 1; i < day.remarksOutput.size(); ++i) {
                    outFile << day.remarksOutput[i] << "\n";
                }
            }
        }
    }
    outFile << "\n";
}

bool IntervalProcessor::loadConfiguration() {
    std::ifstream config_ifs(config_filepath_);
    if (!config_ifs.is_open()) {
        std::cerr << RED_COLOR << "Error: Could not open mapping file: " << config_filepath_ << RESET_COLOR << std::endl;
        return false;
    }
    nlohmann::json j;
    try {
        config_ifs >> j;
    } catch (const std::exception& e) {
        std::cerr << RED_COLOR << "Error parsing main config JSON: " << e.what() << RESET_COLOR << std::endl;
        return false;
    }
    try {
        text_mapping_.clear();
        duration_mappings_.clear();
        text_duration_mapping_.clear();

        if (j.contains("header_order") && j["header_order"].is_array()) {
            header_order_ = j["header_order"].get<std::vector<std::string>>();
        } else {
            throw std::runtime_error("'header_order' key not found or not an array.");
        }

        if (j.contains("remark_prefix") && j["remark_prefix"].is_string()) {
            remark_prefix_ = j["remark_prefix"].get<std::string>();
        } else {
            remark_prefix_ = ""; 
        }

        if (header_order_.empty() || header_order_[0] != "Date:") {
            throw std::runtime_error("'Date:' must be the first item in header_order.");
        }
        if (std::find(header_order_.begin(), header_order_.end(), "Getup:") == header_order_.end() ||
            std::find(header_order_.begin(), header_order_.end(), "Remark:") == header_order_.end()) {
            throw std::runtime_error("'Getup:' and 'Remark:' must be present in header_order.");
        }

        if (j.contains("text_mappings") && j["text_mappings"].is_object()) {
            text_mapping_ = j["text_mappings"].get<std::unordered_map<std::string, std::string>>();
        } else {
            std::cerr << YELLOW_COLOR << "Warning: 'text_mappings' key not found or is not an object in " << config_filepath_ << RESET_COLOR << std::endl;
        }

        if (j.contains("text_duration_mappings") && j["text_duration_mappings"].is_object()) {
            text_duration_mapping_ = j["text_duration_mappings"].get<std::unordered_map<std::string, std::string>>();
        } else {
            std::cerr << YELLOW_COLOR << "Warning: 'text_duration_mappings' key not found or is not an object in " << config_filepath_ << RESET_COLOR << std::endl;
        }
        
        if (j.contains("duration_mappings") && j["duration_mappings"].is_object()) {
            const auto& duration_json = j["duration_mappings"];
            for (auto& [event_key, rules_json] : duration_json.items()) {
                std::vector<DurationRule> rules;
                for (const auto& rule_json : rules_json) {
                    rules.push_back({
                        rule_json.at("less_than_minutes").get<int>(),
                        rule_json.at("value").get<std::string>()
                    });
                }
                std::sort(rules.begin(), rules.end(), [](const DurationRule& a, const DurationRule& b) {
                    return a.less_than_minutes < b.less_than_minutes;
                });
                duration_mappings_[event_key] = rules;
            }
        }
    } catch (const std::exception& e) {
        std::cerr << RED_COLOR << "Error processing content from " << config_filepath_ << ": " << e.what() << RESET_COLOR << std::endl;
        return false;
    }
    return true;
}

std::string IntervalProcessor::formatTime(const std::string& timeStrHHMM) {
    if (timeStrHHMM.length() == 4) {
        return timeStrHHMM.substr(0, 2) + ":" + timeStrHHMM.substr(2, 2);
    }
    return timeStrHHMM;
}

bool IntervalProcessor::isDateLine(const std::string& line) {
    return line.length() == 4 && std::all_of(line.begin(), line.end(), ::isdigit);
}

bool IntervalProcessor::isRemarkLine(const std::string& line) {
    if (remark_prefix_.empty() || line.rfind(remark_prefix_, 0) != 0) {
        return false;
    }
    std::string content = line.substr(remark_prefix_.length());
    content.erase(0, content.find_first_not_of(" \t"));
    return !content.empty();
}

bool IntervalProcessor::parseEventLine(const std::string& line, std::string& outTimeStr, std::string& outDescription) {
    if (line.length() < 5 || !std::all_of(line.begin(), line.begin() + 4, ::isdigit)) {
        return false;
    }
    int hh = std::stoi(line.substr(0, 2));
    int mm = std::stoi(line.substr(2, 2));
    if (hh > 23 || mm > 59) return false;
    outTimeStr = line.substr(0, 4);
    outDescription = line.substr(4);
    return !outDescription.empty();
}

int IntervalProcessor::calculateDurationMinutes(const std::string& startTimeStr, const std::string& endTimeStr) {
    if (startTimeStr.length() != 5 || endTimeStr.length() != 5) return 0;
    try {
        int startHour = std::stoi(startTimeStr.substr(0, 2));
        int startMin = std::stoi(startTimeStr.substr(3, 2));
        int endHour = std::stoi(endTimeStr.substr(0, 2));
        int endMin = std::stoi(endTimeStr.substr(3, 2));
        int startTimeInMinutes = startHour * 60 + startMin;
        int endTimeInMinutes = endHour * 60 + endMin;
        if (endTimeInMinutes < startTimeInMinutes) {
            endTimeInMinutes += 24 * 60;
        }
        return endTimeInMinutes - startTimeInMinutes;
    } catch (const std::exception&) {
        return 0;
    }
}