
#include "IntervalProcessor.h"

#include "common_utils.h" // 获取颜色

#include <iostream>
#include <fstream>
#include <algorithm>
#include <cctype>
#include <ctime>
#include <filesystem>

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


//                  核心转换逻辑 (无变化)                     


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

    while (std::getline(inFile, line)) {
        line.erase(0, line.find_first_not_of(" \t\n\r\f\v"));
        line.erase(line.find_last_not_of(" \t\n\r\f\v") + 1);
        if (line.empty()) continue;

        if (isDateLine(line)) {
            handleDateLine(line, currentDay, previousDay, outFile, year_prefix);
        } else if (isRemarkLine(line)) {
            handleRemarkLine(line, currentDay);
        } else {
            handleEventLine(line, currentDay);
        }
    }

    finalizeAndWriteDay(previousDay, currentDay, outFile);
    
    if (!currentDay.date.empty()) {
        processAndFormatEvents(currentDay);
        writeDayData(outFile, currentDay);
    }
    
    return true;
}



//                      私有辅助函数 

void IntervalProcessor::handleDateLine(const std::string& line, DayData& currentDay, DayData& previousDay, std::ofstream& outFile, const std::string& year_prefix) {
    finalizeAndWriteDay(previousDay, currentDay, outFile);
    previousDay = currentDay;
    currentDay.clear();
    currentDay.date = year_prefix + line;
}

void IntervalProcessor::handleRemarkLine(const std::string& line, DayData& currentDay) {
    if (currentDay.date.empty()) return;
    std::string remark_content = line.substr(remark_prefix_.length());
    remark_content.erase(0, remark_content.find_first_not_of(" \t"));
    if (!remark_content.empty()) {
        currentDay.generalRemarks.push_back(remark_content);
    }
}

void IntervalProcessor::handleEventLine(const std::string& line, DayData& currentDay) {
    if (currentDay.date.empty()) return;
    std::string eventTimeBuffer, eventDescBuffer;
    if (parseEventLine(line, eventTimeBuffer, eventDescBuffer)) {
        // "起床"应该只标记 getupTime，而不应该成为一个活动事件。

        if (eventDescBuffer == "起床" || eventDescBuffer == "醒") {
            if (currentDay.getupTime.empty()) {
                currentDay.getupTime = formatTime(eventTimeBuffer);
            }
        } else {
            if (currentDay.getupTime.empty() && currentDay.rawEvents.empty()) {
                currentDay.isContinuation = true;
            }
        }
        currentDay.rawEvents.push_back({eventTimeBuffer, eventDescBuffer});
    }
}

void IntervalProcessor::processAndFormatEvents(DayData& day) {
    for (const auto& general_remark : day.generalRemarks) {
         day.remarksOutput.push_back(general_remark);
    }
    
    if (day.getupTime.empty() && !day.isContinuation) return;
    
    std::string startTime = day.getupTime;
    for (const auto& rawEvent : day.rawEvents) {
        if (rawEvent.description == "起床" || rawEvent.description == "醒") {
            if (startTime.empty()) {
                 startTime = formatTime(rawEvent.endTimeStr);
            }
            continue;
        }

        std::string formattedEventEndTime = formatTime(rawEvent.endTimeStr);
        std::string mappedDescription = rawEvent.description;
        
        // 步骤 1: 应用常规文本映射
        auto mapIt = text_mapping_.find(mappedDescription); // 注意: 查找键是 mappedDescription
        if (mapIt != text_mapping_.end()) {
            mappedDescription = mapIt->second;
        }

        // 步骤 2: 应用用于时长分类的文本映射 (例如 "饭" -> "meal")
        auto durMapIt = text_duration_mapping_.find(mappedDescription);
        if (durMapIt != text_duration_mapping_.end()) {
            mappedDescription = durMapIt->second;
        }
        
        // 【核心新增逻辑】
        // 步骤 3: 检查映射后的名称是否需要根据时长进一步细分
        auto durationRulesIt = duration_mappings_.find(mappedDescription);
        if (durationRulesIt != duration_mappings_.end()) {
            // 计算时长
            int duration = calculateDurationMinutes(startTime, formattedEventEndTime);
            // 遍历规则，找到第一个匹配的
            for (const auto& rule : durationRulesIt->second) {
                if (duration < rule.less_than_minutes) {
                    mappedDescription = rule.value; // 应用新名称
                    break; // 找到后立即停止
                }
            }
        }

        if (mappedDescription.find("study") != std::string::npos) day.hasStudyActivity = true;
        if (!startTime.empty()) {
            day.remarksOutput.push_back(startTime + "~" + formattedEventEndTime + mappedDescription);
        }
        startTime = formattedEventEndTime;
    }
}

void IntervalProcessor::finalizeAndWriteDay(DayData& dayToFinalize, DayData& nextDay, std::ofstream& outFile) {
    if (dayToFinalize.date.empty()) return;
    processAndFormatEvents(dayToFinalize);
    if (nextDay.isContinuation) {
        dayToFinalize.endsWithSleepNight = false;
        if (!dayToFinalize.rawEvents.empty()) {
            nextDay.getupTime = formatTime(dayToFinalize.rawEvents.back().endTimeStr);
        }
    } else if (!dayToFinalize.isContinuation && !nextDay.getupTime.empty()) {
        if (!dayToFinalize.rawEvents.empty()) {
            std::string lastEventTime = formatTime(dayToFinalize.rawEvents.back().endTimeStr);
            // 应该为"sleep_night"，而不是" sleep_night"，不要加空格
            dayToFinalize.remarksOutput.push_back(lastEventTime + "~" + nextDay.getupTime + "sleep_night");
            dayToFinalize.endsWithSleepNight = true;
        }
    }
    writeDayData(outFile, dayToFinalize);
}



// 输出函数 
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
            // 【核心修正】实现更完善的 Remark 输出逻辑
            
            // 检查原始的通用备注是否存在。
            // generalRemarks 包含了所有 "r "行的内容。
            if (!day.generalRemarks.empty()) {
                // 如果存在通用备注，将第一个备注拼接到 "Remark:" 后面
                outFile << "Remark:" << day.remarksOutput[0] << "\n";
                // 从第二个元素开始循环打印（如果存在的话）
                for (size_t i = 1; i < day.remarksOutput.size(); ++i) {
                    outFile << day.remarksOutput[i] << "\n";
                }
            } else {
                // 如果不存在通用备注，说明 remarksOutput 中全是活动事件
                outFile << "Remark:\n"; // "Remark:" 单独占一行
                // 循环打印所有活动事件
                for (const auto& remark_line : day.remarksOutput) {
                    outFile << remark_line << "\n";
                }
            }
        }
    }
    outFile << "\n"; // 在每个日期块后留一个空行
}


// ###                  配置加载及其他                  ###

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
        std::cerr << RED_COLOR << "Error parsing config JSON: " << e.what() << RESET_COLOR << std::endl;
        return false;
    }
    try {
        text_mapping_.clear();
        duration_mappings_.clear();
        text_duration_mapping_.clear();

        if (j.contains("header_order")) header_order_ = j["header_order"].get<std::vector<std::string>>();
        if (j.contains("remark_prefix")) remark_prefix_ = j["remark_prefix"].get<std::string>();
        if (j.contains("text_mappings")) text_mapping_ = j["text_mappings"].get<std::unordered_map<std::string, std::string>>();
        if (j.contains("text_duration_mappings")) text_duration_mapping_ = j["text_duration_mappings"].get<std::unordered_map<std::string, std::string>>();
        
        // 【核心新增逻辑】加载 duration_mappings
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
                // 【重要】按时长升序排序，确保匹配逻辑正确
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

// --- 新增辅助函数 calculateDurationMinutes ---
int IntervalProcessor::calculateDurationMinutes(const std::string& startTimeStr, const std::string& endTimeStr) {
    if (startTimeStr.length() != 5 || endTimeStr.length() != 5) return 0;
    try {
        int startHour = std::stoi(startTimeStr.substr(0, 2));
        int startMin = std::stoi(startTimeStr.substr(3, 2));
        int endHour = std::stoi(endTimeStr.substr(0, 2));
        int endMin = std::stoi(endTimeStr.substr(3, 2));
        int startTimeInMinutes = startHour * 60 + startMin;
        int endTimeInMinutes = endHour * 60 + endMin;
        // 处理跨天情况
        if (endTimeInMinutes < startTimeInMinutes) {
            endTimeInMinutes += 24 * 60;
        }
        return endTimeInMinutes - startTimeInMinutes;
    } catch (const std::exception&) {
        return 0;
    }
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
    try {
        int hh = std::stoi(line.substr(0, 2));
        int mm = std::stoi(line.substr(2, 2));
        if (hh > 23 || mm > 59) return false;
        outTimeStr = line.substr(0, 4);
        outDescription = line.substr(4);
        return !outDescription.empty();
    } catch (const std::exception&){ return false; }
}
