#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map> 
#include <cctype>    // For std::isdigit
#include <algorithm> // For std::string::find_first_not_of
#include <chrono>    // For high-precision timing
#include <iomanip>   // For std::fixed and std::setprecision
#include <sstream>   // For std::stringstream

// Structure to hold event details (raw from input)
struct RawEvent {
    std::string endTimeStr; // "HHMM" format
    std::string description;
};

// Structure to hold processed daily data
struct DayData {
    std::string date; // "YYYYMMDD" format
    bool hasStudyActivity = false;
    std::string getupTime; // "HH:MM" format
    std::vector<RawEvent> rawEvents;
    std::vector<std::string> remarksOutput;

    // Clear data for the new day
    void clear() {
        date.clear();
        hasStudyActivity = false;
        getupTime.clear();
        rawEvents.clear();
        remarksOutput.clear();
    }
};

// Format HHMM to HH:MM
std::string formatTime(const std::string& timeStr) {
    if (timeStr.length() == 4 &&
        std::isdigit(timeStr[0]) && std::isdigit(timeStr[1]) &&
        std::isdigit(timeStr[2]) && std::isdigit(timeStr[3])) {
        return timeStr.substr(0, 2) + ":" + timeStr.substr(2, 2);
    }
    return timeStr; // Fallback, should not happen with correct parsing
}

// Check if a line is a 4-digit date (MMDD)
bool isDateLine(const std::string& line) {
    if (line.length() != 4) return false;
    for (char c : line) {
        if (!std::isdigit(c)) return false;
    }
    return true;
}

// Parse an event line (HHMM description)
// Returns true if successful, false otherwise.
// Populates timeStr ("HHMM") and description.
bool parseEventLine(const std::string& line, std::string& timeStr, std::string& description) {
    if (line.length() < 5) return false; // Needs HHMM + at least one char for description

    timeStr = line.substr(0, 4);
    for (char c : timeStr) {
        if (!std::isdigit(c)) return false; // Time part must be digits
    }

    // Basic validation for HHMM time values
    try {
        int hh = std::stoi(timeStr.substr(0,2));
        int mm = std::stoi(timeStr.substr(2,2));
        if (hh < 0 || hh > 23 || mm < 0 || mm > 59) return false;
    } catch (const std::out_of_range& oor) {
        return false; // Number out of range
    } catch (const std::invalid_argument& ia) {
        return false; // Invalid argument, not a number
    }

    description = line.substr(4);
    if (description.empty()) return false; // Description cannot be empty

    return true;
}

// Process collected raw events for a day to generate remarks and determine status
// --- 修改：mapping 参数类型改为 std::unordered_map ---
void processDayData(DayData& day, const std::unordered_map<std::string, std::string>& mapping) {
    if (day.date.empty()) {
        return; // No date, nothing to process
    }

    if (day.getupTime.empty() && !day.rawEvents.empty()) {
        return; // No Getup time, cannot generate remarks
    }
    if (day.rawEvents.empty()){ // If no other activities, no remarks to generate
        return;
    }

    std::string currentEventStartTime = day.getupTime; // First activity starts at Getup time

    for (const auto& rawEvent : day.rawEvents) {
        std::string formattedEventStartTime = currentEventStartTime;
        std::string formattedEventEndTime = formatTime(rawEvent.endTimeStr);

        std::string originalDescription = rawEvent.description;
        std::string mappedDescription = originalDescription; // Default to original if not in map

        auto mapIt = mapping.find(originalDescription); // find() works the same
        if (mapIt != mapping.end()) {
            mappedDescription = mapIt->second; // Accessing works the same
        }

        if (mappedDescription.find("study") != std::string::npos) {
            day.hasStudyActivity = true;
        }

        day.remarksOutput.push_back(formattedEventStartTime + "~" + formattedEventEndTime + mappedDescription);
        currentEventStartTime = formattedEventEndTime; // Next event starts when this one ends
    }
}

// Write processed data for a day to the output file
void writeDayData(std::ofstream& outFile, const DayData& day) {
    if (day.date.empty()) {
        return; // Uninitialized day data, do not write
    }

    outFile << "Date:" << day.date << std::endl;
    outFile << "Status:" << (day.hasStudyActivity ? "True" : "False") << std::endl;
    outFile << "Getup:" << day.getupTime << std::endl;
    outFile << "Remark:" << std::endl;
    if (!day.remarksOutput.empty()) {
        for (const auto& remark : day.remarksOutput) {
            outFile << remark << std::endl;
        }
    }
    outFile << std::endl; // Blank line separator after each day's entry
}


int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <input_file.txt>" << std::endl;
        return 1;
    }

    std::string inputFileName = argv[1];
    std::string outputFileName = "output.txt";

    auto startTime = std::chrono::high_resolution_clock::now();

    std::ifstream inFile(inputFileName);
    if (!inFile.is_open()) {
        std::cerr << "Error: Could not open input file " << inputFileName << std::endl;
        return 1;
    }

    std::stringstream buffer;
    buffer << inFile.rdbuf();
    inFile.close();

    std::ofstream outFile(outputFileName);
    if (!outFile.is_open()) {
        std::cerr << "Error: Could not open output file " << outputFileName << std::endl;
        return 1;
    }

    // --- 修改：G_TEXT_MAPPING 类型改为 std::unordered_map ---
    const std::unordered_map<std::string, std::string> G_TEXT_MAPPING = {
        {"word", "study_english_words"},
        {"单词", "study_english_words"},
        {"听力", "study_english_listening"},
        {"文章", "study_english_article"},
        {"timemaster", "code_time-master"},
        {"refactor", "code_refactor"},
        {"休息短", "rest_short"},
        {"休息中", "rest_medium"},
        {"休息长", "rest_long"},
        {"饭中", "meal_medium"},
        {"饭长","meal_long"},
        {"zh", "recreation_zhihu"},
        {"知乎", "recreation_zhihu"},
        {"dy", "recreation_douyin"},
        {"抖音", "recreation_douyin"},
        {"守望先锋", "recreation_game_overwatch"},
        {"皇室", "recreation_game_clash-royale"},
        {"ow", "recreation_game_overwatch"},
        {"bili", "recreation_bilibili"},
        {"mix", "recreation_mix"},
        {"b", "recreation_bilibili"},
        {"电影", "recreation_movie"},
        {"撸", "rest_masturbation"},
        {"school", "other_school"},
        {"有氧", "exercise_cardio"},
        {"无氧", "exercise_anaerobic"},
        {"运动", "exercise_both"},
        {"break", "break_unknown"}
    };

    DayData currentDayData;
    std::string line;
    const std::string YEAR_PREFIX = "2025";

    while (std::getline(buffer, line)) {
        if (line.empty() || line.find_first_not_of(" \t\n\v\f\r") == std::string::npos) {
            continue;
        }

        std::string eventTime, eventDesc;
        if (isDateLine(line)) {
            if (!currentDayData.date.empty()) {
                processDayData(currentDayData, G_TEXT_MAPPING);
                writeDayData(outFile, currentDayData);
            }
            currentDayData.clear();
            currentDayData.date = YEAR_PREFIX + line;
        } else if (parseEventLine(line, eventTime, eventDesc)) {
            if (currentDayData.date.empty()) {
                continue;
            }

            if (eventDesc == "起床") {
                currentDayData.getupTime = formatTime(eventTime);
            } else {
                currentDayData.rawEvents.push_back({eventTime, eventDesc});
            }
        } else {
            // Optional: std::cerr << "Warning: Skipping malformed line: '" << line << "'" << std::endl;
        }
    }

    if (!currentDayData.date.empty()) {
        processDayData(currentDayData, G_TEXT_MAPPING);
        writeDayData(outFile, currentDayData);
    }

    outFile.close();

    auto endTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsedSeconds = endTime - startTime;

    std::cout << "Processing complete. Output written to " << outputFileName << std::endl;
    std::cout << "Time taken: " << std::fixed << std::setprecision(6) << elapsedSeconds.count() << " seconds." << std::endl;

    return 0;
}
