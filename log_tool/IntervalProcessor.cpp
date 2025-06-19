// IntervalProcessor.cpp

#include "IntervalProcessor.h"
#include "SharedUtils.h"
#include <iostream>
#include <fstream>
#include <algorithm>
#include <cctype>
#include <ctime>
#include <filesystem>

// 修改：构造函数接收两个配置文件
IntervalProcessor::IntervalProcessor(const std::string& config_filename, const std::string& header_config_filename)
    : config_filepath_(config_filename), header_config_filepath_(header_config_filename) {
    if (!loadConfiguration()) {
        std::cerr << RED_COLOR << "Error: IntervalProcessor failed to load its configuration." << RESET_COLOR << std::endl;
    }
}

// ... processFile 方法保持不变 ...
bool IntervalProcessor::processFile(const std::string& input_filepath, const std::string& output_filepath) {
    std::ifstream inFile(input_filepath);
    if (!inFile.is_open()) {
        std::cerr << RED_COLOR << "Error: Could not open input file " << input_filepath << RESET_COLOR << std::endl;
        return false;
    }

    std::ofstream outFile(output_filepath);
    if (!outFile.is_open()) {
        std::cerr << RED_COLOR << "Error: Could not open output file " << output_filepath << RESET_COLOR << std::endl;
        inFile.close();
        return false;
    }

    DayData previousDay;
    DayData currentDay;
    std::string line;

    std::time_t now = std::time(nullptr);
    std::tm* ltm = std::localtime(&now);
    std::string year_prefix = std::to_string(1900 + ltm->tm_year);

    std::string eventTimeBuffer;
    std::string eventDescBuffer;

    while (std::getline(inFile, line)) {
        line.erase(0, line.find_first_not_of(" \t\n\r\f\v"));
        line.erase(line.find_last_not_of(" \t\n\r\f\v") + 1);
        if (line.empty()) continue;

        eventTimeBuffer.clear();
        eventDescBuffer.clear();

        if (isDateLine(line)) {
            if (!previousDay.date.empty()) {
                processDayData(previousDay);
                if (!currentDay.getupTime.empty() && !previousDay.rawEvents.empty()) {
                    std::string sleepStartTime = formatTime(previousDay.rawEvents.back().endTimeStr);
                    std::string sleepEndTime = currentDay.getupTime;
                    previousDay.remarksOutput.push_back(sleepStartTime + "~" + sleepEndTime + "sleep_night");
                }
                writeDayData(outFile, previousDay);
            }
            previousDay = currentDay;
            currentDay.clear();
            currentDay.date = year_prefix + line;

        } else if (parseEventLine(line, eventTimeBuffer, eventDescBuffer)) {
            if (currentDay.date.empty()) {
                std::cerr << YELLOW_COLOR << "Warning: Event line '" << line << "' found before any date line." << RESET_COLOR << std::endl;
                continue;
            }
            if (eventDescBuffer == "起床" || eventDescBuffer == "醒") {
                if (currentDay.getupTime.empty()) {
                    currentDay.getupTime = formatTime(eventTimeBuffer);
                } else {
                    currentDay.rawEvents.push_back({eventTimeBuffer, eventDescBuffer});
                }
            } else {
                currentDay.rawEvents.push_back({eventTimeBuffer, eventDescBuffer});
            }
        }
    }

    // After the loop, process the last two days
    if (!previousDay.date.empty()) {
        processDayData(previousDay);
        if (!currentDay.getupTime.empty() && !previousDay.rawEvents.empty()) {
            std::string sleepStartTime = formatTime(previousDay.rawEvents.back().endTimeStr);
            std::string sleepEndTime = currentDay.getupTime;
            previousDay.remarksOutput.push_back(sleepStartTime + "~" + sleepEndTime + "sleep_night");
        }
        writeDayData(outFile, previousDay);
    }
    if (!currentDay.date.empty()) {
        processDayData(currentDay);
        writeDayData(outFile, currentDay);
    }

    inFile.close();
    outFile.close();
    return true;
}

// 修改：加载两个配置文件
bool IntervalProcessor::loadConfiguration() {
    // 1. 加载文本映射配置
    std::ifstream mapping_ifs(config_filepath_);
    if (!mapping_ifs.is_open()) {
        std::cerr << RED_COLOR << "Error: Could not open mapping file: " << config_filepath_ << RESET_COLOR << std::endl;
        return false;
    }
    try {
        nlohmann::json j;
        mapping_ifs >> j;
        text_mapping_ = j.get<std::unordered_map<std::string, std::string>>();
    } catch (const std::exception& e) {
        std::cerr << RED_COLOR << "Error processing mapping JSON: " << e.what() << RESET_COLOR << std::endl;
        return false;
    }

    // 2. 加载头部顺序配置
    std::ifstream header_ifs(header_config_filepath_);
    if (!header_ifs.is_open()) {
        std::cerr << RED_COLOR << "Error: Could not open header format file: " << header_config_filepath_ << RESET_COLOR << std::endl;
        return false;
    }
    try {
        nlohmann::json j;
        header_ifs >> j;
        if (j.contains("header_order") && j["header_order"].is_array()) {
            header_order_ = j["header_order"].get<std::vector<std::string>>();
        } else {
            throw std::runtime_error("'header_order' key not found or not an array.");
        }
        // 验证必须的字段
        if (header_order_.empty() || header_order_[0] != "Date:") {
            throw std::runtime_error("'Date:' must be the first item in header_order.");
        }
        if (std::find(header_order_.begin(), header_order_.end(), "Getup:") == header_order_.end() ||
            std::find(header_order_.begin(), header_order_.end(), "Remark:") == header_order_.end()) {
            throw std::runtime_error("'Getup:' and 'Remark:' must be present in header_order.");
        }
    } catch (const std::exception& e) {
        std::cerr << RED_COLOR << "Error processing header format JSON: " << e.what() << RESET_COLOR << std::endl;
        return false;
    }

    return true;
}

// ... DayData::clear(), formatTime(), isDateLine(), parseEventLine(), processDayData() 保持不变 ...
void IntervalProcessor::DayData::clear() {
    date.clear();
    hasStudyActivity = false;
    getupTime.clear();
    rawEvents.clear();
    remarksOutput.clear();
}

std::string IntervalProcessor::formatTime(const std::string& timeStrHHMM) {
    if (timeStrHHMM.length() == 4) {
        return timeStrHHMM.substr(0, 2) + ":" + timeStrHHMM.substr(2, 2);
    }
    return timeStrHHMM; // Return original if format is unexpected
}

bool IntervalProcessor::isDateLine(const std::string& line) {
    return line.length() == 4 && std::all_of(line.begin(), line.end(), ::isdigit);
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

void IntervalProcessor::processDayData(DayData& day) {
    if (day.date.empty() || day.getupTime.empty() || day.rawEvents.empty()) {
        return;
    }

    std::string currentEventIntervalStartTime = day.getupTime;
    for (const auto& rawEvent : day.rawEvents) {
        std::string formattedEventEndTime = formatTime(rawEvent.endTimeStr);
        std::string mappedDescription = rawEvent.description;

        auto mapIt = text_mapping_.find(rawEvent.description);
        if (mapIt != text_mapping_.end()) {
            mappedDescription = mapIt->second;
        }

        if (mappedDescription.find("study") != std::string::npos) {
            day.hasStudyActivity = true;
        }

        day.remarksOutput.push_back(currentEventIntervalStartTime + "~" + formattedEventEndTime + mappedDescription);
        currentEventIntervalStartTime = formattedEventEndTime;
    }
}


// 重写：根据 header_order_ 动态生成输出文件
void IntervalProcessor::writeDayData(std::ofstream& outFile, const DayData& day) {
    if (day.date.empty()) return;

    for (const auto& header : header_order_) {
        if (header == "Date:") {
            outFile << "Date:" << day.date << "\n";
        } else if (header == "Status:") {
            outFile << "Status:" << (day.hasStudyActivity ? "True" : "False") << "\n";
        } else if (header == "Sleep:") {
            outFile << "Sleep:True\n"; // 默认值为 True
        } else if (header == "Getup:") {
            outFile << "Getup:" << (day.getupTime.empty() ? "00:00" : day.getupTime) << "\n";
        } else if (header == "Remark:") {
            outFile << "Remark:\n";
            for (const auto& remark : day.remarksOutput) {
                outFile << remark << "\n";
            }
        }
    }
    outFile << "\n"; // 每个日期块后加一个空行
}