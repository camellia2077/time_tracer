#include "IntervalProcessor.h"
#include "SharedUtils.h"
#include <iostream>
#include <fstream>
#include <algorithm>
#include <cctype>
#include <ctime>
#include <filesystem>

namespace fs = std::filesystem;

// --- Constructor ---
IntervalProcessor::IntervalProcessor(const std::string& config_filename)
    : config_filepath_(config_filename) {
    if (!loadTextMapping()) {
        std::cerr << RED_COLOR << "Error: IntervalProcessor failed to load its configuration." << RESET_COLOR << std::endl;
        // The object can still be created, but processFile will likely fail or work with no mappings.
    }
}

// --- Public Methods ---
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


// --- Private Helper Methods ---

bool IntervalProcessor::loadTextMapping() {
    std::ifstream ifs(config_filepath_);
    if (!ifs.is_open()) {
        std::cerr << RED_COLOR << "Error: Could not open mapping file: " << config_filepath_ << RESET_COLOR << std::endl;
        return false;
    }

    nlohmann::json j;
    try {
        ifs >> j;
        if (j.is_object()) {
            text_mapping_ = j.get<std::unordered_map<std::string, std::string>>();
        } else {
            std::cerr << RED_COLOR << "Error: Root of JSON in " << config_filepath_ << " is not an object." << RESET_COLOR << std::endl;
            return false;
        }
    } catch (const nlohmann::json::parse_error& e) {
        std::cerr << RED_COLOR << "Error parsing mapping JSON: " << e.what() << RESET_COLOR << std::endl;
        return false;
    } catch (const nlohmann::json::type_error& e) {
        std::cerr << RED_COLOR << "Error: Type mismatch in JSON config: " << e.what() << RESET_COLOR << std::endl;
        return false;
    }
    return true;
}

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

void IntervalProcessor::writeDayData(std::ofstream& outFile, const DayData& day) {
    if (day.date.empty()) return;

    outFile << "Date:" << day.date << "\n";
    outFile << "Status:" << (day.hasStudyActivity ? "True" : "False") << "\n";
    outFile << "Getup:" << day.getupTime << "\n";
    outFile << "Remark:\n";
    for (const auto& remark : day.remarksOutput) {
        outFile << remark << "\n";
    }
    outFile << "\n";
}