// IntervalProcessor.cpp

#include "IntervalProcessor.h"
#include "SharedUtils.h"
#include <iostream>
#include <fstream>
#include <algorithm>
#include <cctype>
#include <ctime>
#include <filesystem>

IntervalProcessor::IntervalProcessor(const std::string& config_filename, const std::string& header_config_filename)
    : config_filepath_(config_filename), header_config_filepath_(header_config_filename) {
    if (!loadConfiguration()) {
        std::cerr << RED_COLOR << "Error: IntervalProcessor failed to load its configuration." << RESET_COLOR << std::endl;
    }
}

void IntervalProcessor::DayData::clear() {
    date.clear();
    hasStudyActivity = false;
    endsWithSleepNight = false; 
    getupTime.clear();
    rawEvents.clear();
    remarksOutput.clear();
    isContinuation = false; 
}

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
    std::string eventTimeBuffer, eventDescBuffer;

    auto process_day_events = [&](DayData& day) {
        if (day.getupTime.empty() && !day.isContinuation) return;
        if (day.getupTime.empty() && day.isContinuation) {
            if(day.getupTime.empty()) return;
        }

        std::string startTime = day.getupTime;
        for (const auto& rawEvent : day.rawEvents) {
            std::string formattedEventEndTime = formatTime(rawEvent.endTimeStr);
            std::string mappedDescription = rawEvent.description;

            auto mapIt = text_mapping_.find(rawEvent.description);
            if (mapIt != text_mapping_.end()) mappedDescription = mapIt->second;

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
            if (mappedDescription.find("study") != std::string::npos) day.hasStudyActivity = true;
            day.remarksOutput.push_back(startTime + "~" + formattedEventEndTime + mappedDescription);
            startTime = formattedEventEndTime;
        }
    };


    while (std::getline(inFile, line)) {
        line.erase(0, line.find_first_not_of(" \t\n\r\f\v"));
        line.erase(line.find_last_not_of(" \t\n\r\f\v") + 1);
        if (line.empty()) continue;

        if (isDateLine(line)) {
            if (!previousDay.date.empty()) {
                process_day_events(previousDay);

                if (currentDay.isContinuation) {
                    previousDay.endsWithSleepNight = false; 
                    if (!previousDay.rawEvents.empty()) {
                        currentDay.getupTime = formatTime(previousDay.rawEvents.back().endTimeStr);
                    }
                // --- MODIFICATION START: Added !previousDay.isContinuation check ---
                } else if (!previousDay.isContinuation && !currentDay.getupTime.empty()) {
                // --- MODIFICATION END ---
                    if (!previousDay.rawEvents.empty()) {
                        std::string sleepStartTime = formatTime(previousDay.rawEvents.back().endTimeStr);
                        previousDay.remarksOutput.push_back(sleepStartTime + "~" + currentDay.getupTime + "sleep_night");
                        previousDay.endsWithSleepNight = true;
                    }
                }
                
                writeDayData(outFile, previousDay);
            }
            previousDay = currentDay;
            currentDay.clear();
            currentDay.date = year_prefix + line;
        } else if (parseEventLine(line, eventTimeBuffer, eventDescBuffer)) {
            if (currentDay.date.empty()) continue;

            if (eventDescBuffer == "起床" || eventDescBuffer == "醒") {
                if (currentDay.getupTime.empty()) currentDay.getupTime = formatTime(eventTimeBuffer);
                else currentDay.rawEvents.push_back({eventTimeBuffer, eventDescBuffer});
            } else {
                if (currentDay.getupTime.empty() && currentDay.rawEvents.empty()) {
                    currentDay.isContinuation = true; 
                }
                currentDay.rawEvents.push_back({eventTimeBuffer, eventDescBuffer});
            }
        }
    }

    if (!previousDay.date.empty()) {
        process_day_events(previousDay);
        // --- MODIFICATION START: Added !previousDay.isContinuation check ---
        if (currentDay.isContinuation) {
        // --- MODIFICATION END ---
            previousDay.endsWithSleepNight = false;
            if (!previousDay.rawEvents.empty()) {
                currentDay.getupTime = formatTime(previousDay.rawEvents.back().endTimeStr);
            }
        } else if (!previousDay.isContinuation && !currentDay.getupTime.empty()) {
             if (!previousDay.rawEvents.empty()) {
                 std::string sleepStartTime = formatTime(previousDay.rawEvents.back().endTimeStr);
                 previousDay.remarksOutput.push_back(sleepStartTime + "~" + currentDay.getupTime + "sleep_night");
                 previousDay.endsWithSleepNight = true;
            }
        }
        writeDayData(outFile, previousDay);
    }
    if (!currentDay.date.empty()) {
        process_day_events(currentDay);
        writeDayData(outFile, currentDay);
    }
    
    inFile.close();
    outFile.close();
    return true;
}

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
            outFile << "Remark:\n";
            for (const auto& remark : day.remarksOutput) {
                outFile << remark << "\n";
            }
        }
    }
    outFile << "\n"; 
}

bool IntervalProcessor::loadConfiguration() {
    std::ifstream main_config_ifs(config_filepath_);
    if (!main_config_ifs.is_open()) {
        std::cerr << RED_COLOR << "Error: Could not open mapping file: " << config_filepath_ << RESET_COLOR << std::endl;
        return false;
    }
    nlohmann::json j;
    try {
        main_config_ifs >> j;
    } catch (const std::exception& e) {
        std::cerr << RED_COLOR << "Error parsing main config JSON: " << e.what() << RESET_COLOR << std::endl;
        return false;
    }
    try {
        text_mapping_.clear();
        duration_mappings_.clear();
        if (j.contains("text_mappings") && j["text_mappings"].is_object()) {
            text_mapping_ = j["text_mappings"].get<std::unordered_map<std::string, std::string>>();
        } else {
            std::cerr << YELLOW_COLOR << "Warning: 'text_mappings' key not found or is not an object in " << config_filepath_ << RESET_COLOR << std::endl;
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
    std::ifstream header_ifs(header_config_filepath_);
    if (!header_ifs.is_open()) {
        std::cerr << RED_COLOR << "Error: Could not open header format file: " << header_config_filepath_ << RESET_COLOR << std::endl;
        return false;
    }
    try {
        nlohmann::json hj;
        header_ifs >> hj;
        if (hj.contains("header_order") && hj["header_order"].is_array()) {
            header_order_ = hj["header_order"].get<std::vector<std::string>>();
        } else {
            throw std::runtime_error("'header_order' key not found or not an array.");
        }
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

std::string IntervalProcessor::formatTime(const std::string& timeStrHHMM) {
    if (timeStrHHMM.length() == 4) {
        return timeStrHHMM.substr(0, 2) + ":" + timeStrHHMM.substr(2, 2);
    }
    return timeStrHHMM;
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