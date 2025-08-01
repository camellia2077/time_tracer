#include "common/pch.h"
// reprocessing/input_transfer/IntervalProcessor.cpp

#include "IntervalProcessor.h"

#include "common/common_utils.h"

#include "reprocessing/input_transfer/internal/InputData.h"
#include "reprocessing/input_transfer/internal/IntervalConverter.h"

#include <fstream>
#include <iostream>
#include <algorithm>
#include <cctype>

IntervalProcessor::IntervalProcessor(const std::string& config_filename) {
    if (!config_.load(config_filename)) {
        throw std::runtime_error("Failed to load IntervalProcessor configuration.");
    }
}

namespace {
    // 匿名空间中的辅助函数
    void writeInputData(std::ofstream& outFile, const InputData& day, const IntervalProcessorConfig& config) {
        if (day.date.empty()) return;
        for (const auto& header : config.getHeaderOrder()) {
            if (header == "Date:") {
                outFile << "Date:" << day.date << "\n";
            } else if (header == "Status:") {
                outFile << "Status:" << (day.hasStudyActivity ? "True" : "False") << "\n";
            } else if (header == "Sleep:") {
                // -- FIX START --
                bool sleepStatus = day.isContinuation ? false : day.endsWithSleepNight;
                outFile << "Sleep:" << (sleepStatus ? "True" : "False") << "\n";
                // -- FIX END --
            } else if (header == "Getup:") {
                outFile << "Getup:" << (day.isContinuation ? "Null" : (day.getupTime.empty() ? "00:00" : day.getupTime)) << "\n";
            } else if (header == "Remark:") {
                if (!day.generalRemarks.empty()) {
                    outFile << "Remark:" << day.remarksOutput[0] << "\n";
                    for (size_t i = 1; i < day.remarksOutput.size(); ++i) {
                        outFile << day.remarksOutput[i] << "\n";
                    }
                } else {
                    outFile << "Remark:\n";
                    for (const auto& remark_line : day.remarksOutput) {
                        outFile << remark_line << "\n";
                    }
                }
            }
        }
        outFile << "\n";
    }
    std::string formatTime(const std::string& timeStrHHMM) {
        return (timeStrHHMM.length() == 4) ? timeStrHHMM.substr(0, 2) + ":" + timeStrHHMM.substr(2, 2) : timeStrHHMM;
    }
}

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

    IntervalConverter converter(config_);
    const std::string& remark_prefix = config_.getRemarkPrefix();
    InputData previousDay, currentDay;
    std::string line;

    auto finalizeAndWrite = [&](InputData& dayToFinalize, InputData& nextDay) {
        if (dayToFinalize.date.empty()) return;
        converter.transform(dayToFinalize);
        if (nextDay.isContinuation) {
            dayToFinalize.endsWithSleepNight = false;
            if (!dayToFinalize.rawEvents.empty()) {
                nextDay.getupTime = formatTime(dayToFinalize.rawEvents.back().endTimeStr);
            }
        } else if (!dayToFinalize.isContinuation && !nextDay.getupTime.empty()) {
            if (!dayToFinalize.rawEvents.empty()) {
                std::string lastEventTime = formatTime(dayToFinalize.rawEvents.back().endTimeStr);
                dayToFinalize.remarksOutput.push_back(lastEventTime + "~" + nextDay.getupTime + "sleep_night");
                dayToFinalize.endsWithSleepNight = true;
            }
        }
        writeInputData(outFile, dayToFinalize, config_);
    };

    while (std::getline(inFile, line)) {
        line.erase(0, line.find_first_not_of(" \t\n\r\f\v"));
        line.erase(line.find_last_not_of(" \t\n\r\f\v") + 1);
        if (line.empty()) continue;

        if (line.length() == 4 && std::all_of(line.begin(), line.end(), ::isdigit)) {
            finalizeAndWrite(previousDay, currentDay);
            previousDay = currentDay;
            currentDay.clear();
            currentDay.date = year_prefix + line;
        } else if (!remark_prefix.empty() && line.rfind(remark_prefix, 0) == 0) {
            if (!currentDay.date.empty()) {
                 currentDay.generalRemarks.push_back(line.substr(remark_prefix.length()));
            }
        } else {
             if (!currentDay.date.empty() && line.length() >= 5 && std::all_of(line.begin(), line.begin() + 4, ::isdigit)) {
                std::string timeStr = line.substr(0, 4);
                std::string desc = line.substr(4);
                if (desc == "起床" || desc == "醒") {
                    if (currentDay.getupTime.empty()) currentDay.getupTime = formatTime(timeStr);
                } else {
                    if (currentDay.getupTime.empty() && currentDay.rawEvents.empty()) currentDay.isContinuation = true;
                }
                currentDay.rawEvents.push_back({timeStr, desc});
            }
        }
    }

    finalizeAndWrite(previousDay, currentDay);
    if (!currentDay.date.empty()) {
        InputData finalDay;
        converter.transform(currentDay);
        writeInputData(outFile, currentDay, config_);
    }
    
    return true;
}