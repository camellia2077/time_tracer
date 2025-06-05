#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <cctype>
#include <algorithm>
#include <chrono>
#include <iomanip>
#include <filesystem>
#include <ctime>   // Required for std::localtime, std::tm
#include <sstream> // Required for std::ostringstream

#include "json.hpp"
namespace fs = std::filesystem;

// Structure to hold event details (raw from input)
struct RawEvent {
    std::string endTimeStr; // "HHMM" format
    std::string description;
};

// Structure to hold processed daily data
struct DayData {
    std::string date; // "YYYYMMDD" format
    bool hasStudyActivity = false;
    std::string getupTime; // "HH:MM" format, or empty
    std::vector<RawEvent> rawEvents;
    std::vector<std::string> remarksOutput;

    void clear() {
        date.clear();
        hasStudyActivity = false;
        getupTime.clear();
        rawEvents.clear();
        remarksOutput.clear();
    }
};

// Function to load the text mapping from a JSON file
std::unordered_map<std::string, std::string> load_text_mapping(const std::string& filename) {
    std::ifstream ifs(filename);
    if (!ifs.is_open()) {
        std::cerr << "Error: Could not open mapping file: " << filename << std::endl;
        return {}; // Return an empty map on error
    }

    nlohmann::json j;
    try {
        ifs >> j; // Parse the JSON from the input stream
    } catch (nlohmann::json::parse_error& e) {
        std::cerr << "Error: Failed to parse mapping JSON from " << filename << "\n"
                  << "Message: " << e.what() << "\n"
                  << "Byte position: " << e.byte << std::endl;
        return {}; // Return an empty map on error
    }
    ifs.close();

    // Check if the parsed JSON is an object (map-like structure)
    if (!j.is_object()) {
        std::cerr << "Error: Root of JSON in " << filename << " is not an object." << std::endl;
        return {};
    }

    std::unordered_map<std::string, std::string> mapping;
    try {
        mapping = j.get<std::unordered_map<std::string, std::string>>();
    } catch (nlohmann::json::type_error& e) {
         std::cerr << "Error: Type mismatch when converting JSON to map from " << filename << ".\n"
                   << "Ensure all values in the JSON object are strings.\n"
                   << "Message: " << e.what() << std::endl;
         return {};
    }
    return mapping;
}

// Format HHMM to HH:MM
std::string formatTime(const std::string& timeStrHHMM) {
    if (timeStrHHMM.length() == 4) {
        std::string formattedTime = "00:00"; 
        formattedTime[0] = timeStrHHMM[0];
        formattedTime[1] = timeStrHHMM[1];
        formattedTime[3] = timeStrHHMM[2];
        formattedTime[4] = timeStrHHMM[3];
        return formattedTime;
    }
    return timeStrHHMM;
}

// Check if a line is a 4-digit date (MMDD)
bool isDateLine(const std::string& line) {
    if (line.length() != 4) return false;
    return std::all_of(line.begin(), line.end(), ::isdigit);
}

// Parse an event line (HHMMdescription)
bool parseEventLine(const std::string& line, std::string& outTimeStr, std::string& outDescription) {
    if (line.length() < 5) return false; 

    if (!std::isdigit(line[0]) || !std::isdigit(line[1]) ||
        !std::isdigit(line[2]) || !std::isdigit(line[3])) {
        return false;
    }

    int hh = (line[0] - '0') * 10 + (line[1] - '0');
    int mm = (line[2] - '0') * 10 + (line[3] - '0');

    if (hh > 23 || mm > 59) return false;
    outTimeStr.assign(line.data(), 4);
    outDescription.assign(line.data() + 4, line.length() - 4);
    if (outDescription.empty()) return false;
    return true;
}

// Process collected raw events for a day
void processDayData(DayData& day, const std::unordered_map<std::string, std::string>& mapping) {
    if (day.date.empty()) {
        return;
    }

    if (day.getupTime.empty() && !day.rawEvents.empty()) {
        return;
    }
    if (day.rawEvents.empty()){
        // If there are no raw events, but there was a getup time,
        // remarksOutput will remain empty, which is fine.
        // If sleep_night is added later, it will be the only remark.
        return;
    }

    std::string currentEventIntervalStartTime = day.getupTime; 

    for (const auto& rawEvent : day.rawEvents) {
        std::string formattedEventEndTime = formatTime(rawEvent.endTimeStr);
        std::string originalDescription = rawEvent.description;
        std::string mappedDescription = originalDescription;

        auto mapIt = mapping.find(originalDescription);
        if (mapIt != mapping.end()) {
            mappedDescription = mapIt->second;
        }

        if (mappedDescription.find("study") != std::string::npos) {
            day.hasStudyActivity = true;
        }

        std::string remark;
        remark.reserve(currentEventIntervalStartTime.length() + 1 + formattedEventEndTime.length() + mappedDescription.length());
        remark.append(currentEventIntervalStartTime);
        remark.append("~");
        remark.append(formattedEventEndTime);
        remark.append(mappedDescription);
        day.remarksOutput.push_back(std::move(remark)); 

        currentEventIntervalStartTime = formattedEventEndTime;
    }
}

// Write processed data for a day
void writeDayData(std::ofstream& outFile, const DayData& day) {
    if (day.date.empty()) {
        return;
    }

    outFile << "Date:" << day.date << "\n";
    outFile << "Status:" << (day.hasStudyActivity ? "True" : "False") << "\n";
    outFile << "Getup:" << day.getupTime << "\n";
    outFile << "Remark:\n";
    if (!day.remarksOutput.empty()) {
        for (const auto& remark : day.remarksOutput) {
            outFile << remark << "\n";
        }
    }
    outFile << "\n"; 
}


int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <input_file.txt>" << std::endl;
        return 1;
    }

    std::ios_base::sync_with_stdio(false);

    std::string inputFileName = argv[1];
    fs::path inputPath(inputFileName);
    std::string baseName = inputPath.filename().string();
    std::string outputFileName = "Duration_" + baseName;
    std::string mappingFileName = "interval_processor_config.json";

    auto overallStartTime = std::chrono::high_resolution_clock::now();

    std::unordered_map<std::string, std::string> G_TEXT_MAPPING = load_text_mapping(mappingFileName);
    if (G_TEXT_MAPPING.empty() && fs::exists(mappingFileName)) { // Check if file exists but map is empty due to parse error
        std::cerr << "Warning: Text mapping loaded from " << mappingFileName << " is empty. Proceeding without mappings." << std::endl;
        // Allow proceeding if mapping is empty but file exists (e.g. intentionally empty JSON object)
        // If file doesn't exist, load_text_mapping already printed an error.
    } else if (G_TEXT_MAPPING.empty() && !fs::exists(mappingFileName)) {
         std::cerr << "Error: Text mapping file " << mappingFileName << " not found. Exiting." << std::endl;
        return 1; // Exit if mapping file is crucial and not found
    }


    std::ifstream inFile(inputFileName);
    if (!inFile.is_open()) {
        std::cerr << "Error: Could not open input file " << inputFileName << std::endl;
        return 1;
    }

    std::ofstream outFile(outputFileName);
    if (!outFile.is_open()) {
        std::cerr << "Error: Could not open output file " << outputFileName << std::endl;
        inFile.close();
        return 1;
    }

    DayData previousDay; // Stores the data for the day before the current one being parsed (N-1)
    DayData currentDay;  // Stores data for the day currently being parsed (N)
    std::string line;
    
    // --- Modification Start ---
    // Get current year
    std::time_t now = std::time(nullptr);
    std::tm* ltm = std::localtime(&now);
    std::string YEAR_PREFIX = std::to_string(1900 + ltm->tm_year);
    // --- Modification End ---


    std::string eventTimeBuffer;
    std::string eventDescBuffer;

    while (std::getline(inFile, line)) {
        // Basic line cleanup (remove leading/trailing whitespace)
        line.erase(0, line.find_first_not_of(" \t\n\r\f\v"));
        line.erase(line.find_last_not_of(" \t\n\r\f\v") + 1);

        if (line.empty()) {
            continue;
        }

        eventTimeBuffer.clear();
        eventDescBuffer.clear();

        if (isDateLine(line)) { // Marks the beginning of a new day's data (currentDay)
                                // previousDay's data is now complete regarding its own events.
            if (!previousDay.date.empty()) {
                // Process previousDay's own events first
                processDayData(previousDay, G_TEXT_MAPPING);

                // Add sleep_night to previousDay if currentDay has a getup time
                // and previousDay had actual activities.
                if (!currentDay.getupTime.empty() && !previousDay.rawEvents.empty()) {
                    std::string sleepStartTime = formatTime(previousDay.rawEvents.back().endTimeStr);
                    std::string sleepEndTime = currentDay.getupTime; // Already HH:MM

                    std::string sleepRemark;
                    sleepRemark.reserve(sleepStartTime.length() + 1 + sleepEndTime.length() + 11 /*sleep_night*/);
                    sleepRemark.append(sleepStartTime);
                    sleepRemark.append("~");
                    sleepRemark.append(sleepEndTime);
                    sleepRemark.append("sleep_night");
                    previousDay.remarksOutput.push_back(std::move(sleepRemark));
                }
                writeDayData(outFile, previousDay);
            }

            // Shift days: currentDay becomes previousDay, and currentDay is reset for the new date
            previousDay = currentDay;
            currentDay.clear();
            currentDay.date = YEAR_PREFIX + line; //

        } else if (parseEventLine(line, eventTimeBuffer, eventDescBuffer)) {
            if (currentDay.date.empty()) { // Should not happen if date lines are always first for a day
                std::cerr << "Warning: Event line encountered before a date. Line: '" << line << "'" << std::endl;
                continue;
            }

            // Check for "起床" (get up) or "醒" (wake)
            if (eventDescBuffer == "起床" || eventDescBuffer == "醒") {
                if (currentDay.getupTime.empty()) { // Only set the first getup time for the day
                    currentDay.getupTime = formatTime(eventTimeBuffer);
                } else {
                    // If getup time is already set, treat subsequent "起床" or "醒" as regular events
                    currentDay.rawEvents.push_back({eventTimeBuffer, eventDescBuffer});
                }
            } else {
                currentDay.rawEvents.push_back({eventTimeBuffer, eventDescBuffer});
            }
        } else {
            // Optional: std::cerr << "Warning: Skipping malformed line: '" << line << "'" << std::endl;
        }
    }

    // After the loop, process the last two days held in previousDay and currentDay

    // Process previousDay (which is the second to last day from the input)
    if (!previousDay.date.empty()) {
        processDayData(previousDay, G_TEXT_MAPPING);

        // Add sleep_night to previousDay if currentDay (the last day) has a getup time
        // and previousDay had actual activities.
        if (!currentDay.getupTime.empty() && !previousDay.rawEvents.empty()) {
            std::string sleepStartTime = formatTime(previousDay.rawEvents.back().endTimeStr);
            std::string sleepEndTime = currentDay.getupTime;

            std::string sleepRemark;
            sleepRemark.reserve(sleepStartTime.length() + 1 + sleepEndTime.length() + 11);
            sleepRemark.append(sleepStartTime);
            sleepRemark.append("~");
            sleepRemark.append(sleepEndTime);
            sleepRemark.append("sleep_night");
            previousDay.remarksOutput.push_back(std::move(sleepRemark));
        }
        writeDayData(outFile, previousDay);
    }

    // Process currentDay (the very last day from the input)
    // It won't have a "sleep_night" added to it by this new logic, as there's no subsequent day.
    if (!currentDay.date.empty()) {
        processDayData(currentDay, G_TEXT_MAPPING);
        writeDayData(outFile, currentDay);
    }

    inFile.close();
    outFile.close();

    auto overallEndTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsedSeconds = overallEndTime - overallStartTime;

    std::cout << "Processing complete. Output written to " << outputFileName << std::endl;
    if (fs::exists(mappingFileName)) {
        std::cout << "Text mapping loaded from " << mappingFileName << std::endl;
    } else {
        std::cout << "Text mapping file " << mappingFileName << " not found or not used." << std::endl;
    }
    std::cout << "Time taken: " << std::fixed << std::setprecision(6) << elapsedSeconds.count() << " seconds." << std::endl;

    return 0;
}
