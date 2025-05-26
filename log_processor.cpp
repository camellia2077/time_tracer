#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <cctype>    // For std::isdigit, ::isspace
#include <algorithm> // For std::all_of
#include <chrono>    // For high-precision timing
#include <iomanip>   // For std::fixed and std::setprecision
// <sstream> is no longer needed for buffering the whole file

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

// Format HHMM to HH:MM
// Assumes timeStrHHMM is a valid 4-digit string.
std::string formatTime(const std::string& timeStrHHMM) {
    if (timeStrHHMM.length() == 4) {
        std::string formattedTime = "00:00"; // Pre-allocate with "HH:MM" structure
        formattedTime[0] = timeStrHHMM[0];
        formattedTime[1] = timeStrHHMM[1];
        // formattedTime[2] is already ':'
        formattedTime[3] = timeStrHHMM[2];
        formattedTime[4] = timeStrHHMM[3];
        return formattedTime;
    }
    // Fallback for unexpected cases
    return timeStrHHMM; 
}

// Check if a line is a 4-digit date (MMDD)
bool isDateLine(const std::string& line) {
    if (line.length() != 4) return false;
    return std::all_of(line.begin(), line.end(), ::isdigit);
}

// Parse an event line (HHMMdescription)
// Populates outTimeStr ("HHMM") and outDescription.
bool parseEventLine(const std::string& line, std::string& outTimeStr, std::string& outDescription) {
    if (line.length() < 5) return false; // Minimum HHMM + 1 char for description

    // Validate first 4 chars are digits for time without creating a substring yet
    if (!std::isdigit(line[0]) || !std::isdigit(line[1]) ||
        !std::isdigit(line[2]) || !std::isdigit(line[3])) {
        return false;
    }

    // Efficiently convert and validate HHMM time values
    int hh = (line[0] - '0') * 10 + (line[1] - '0');
    int mm = (line[2] - '0') * 10 + (line[3] - '0');

    if (hh > 23 || mm > 59) return false;

    // Use assign with pointer and length to avoid intermediate substr objects for performance
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

    // Preserving original C++ logic: If Getup time is missing, but there are events, 
    // do not generate remarks (differs from the Python script).
    if (day.getupTime.empty() && !day.rawEvents.empty()) {
        return;
    }
    if (day.rawEvents.empty()){
        return;
    }

    std::string currentEventIntervalStartTime = day.getupTime; // Already "HH:MM" or empty

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
        day.remarksOutput.push_back(std::move(remark)); // Use std::move

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
    outFile << "\n"; // Blank line separator
}


int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <input_file.txt>" << std::endl;
        return 1;
    }

    // Optimizations for C++ standard streams (though primarily using fstream here)
    std::ios_base::sync_with_stdio(false);
    // std::cin.tie(NULL); // Not using cin, but good practice if cin were used

    std::string inputFileName = argv[1];
    std::string outputFileName = "output.txt"; 

    auto overallStartTime = std::chrono::high_resolution_clock::now();

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
    
    const std::unordered_map<std::string, std::string> G_TEXT_MAPPING = {
        {"word", "study_english_words"}, {"单词", "study_english_words"},
        {"听力", "study_english_listening"}, {"文章", "study_english_article"},
        {"timemaster", "code_time-master"}, {"refactor", "code_refactor"},
        {"休息短", "rest_short"}, {"休息中", "rest_medium"}, {"休息长", "rest_long"},
        {"饭中", "meal_medium"}, {"饭长","meal_long"},
        {"zh", "recreation_zhihu"}, {"知乎", "recreation_zhihu"},
        {"dy", "recreation_douyin"}, {"抖音", "recreation_douyin"},
        {"守望先锋", "recreation_game_overwatch"}, {"皇室", "recreation_game_clash-royale"},
        {"ow", "recreation_game_overwatch"}, {"bili", "recreation_bilibili"},
        {"mix", "recreation_mix"}, {"b", "recreation_bilibili"},
        {"电影", "recreation_movie"}, {"撸", "rest_masturbation"},
        {"school", "other_school"}, {"有氧", "exercise_cardio"},
        {"无氧", "exercise_anaerobic"}, {"运动", "exercise_both"},
        {"break", "break_unknown"}
    };

    DayData currentDayData;
    std::string line;
    const std::string YEAR_PREFIX = "2025";
    
    // Pre-allocate strings for parsing results to reduce reallocations in the loop
    std::string eventTimeBuffer; 
    std::string eventDescBuffer;

    // Direct line-by-line reading from file stream
    while (std::getline(inFile, line)) {
        if (line.empty() || std::all_of(line.begin(), line.end(), ::isspace)) {
            continue;
        }
        
        eventTimeBuffer.clear(); // Reuse buffers
        eventDescBuffer.clear();

        if (isDateLine(line)) {
            if (!currentDayData.date.empty()) {
                processDayData(currentDayData, G_TEXT_MAPPING);
                writeDayData(outFile, currentDayData);
            }
            currentDayData.clear();
            currentDayData.date = YEAR_PREFIX + line;
        } else if (parseEventLine(line, eventTimeBuffer, eventDescBuffer)) {
            if (currentDayData.date.empty()) {
                // std::cerr << "Warning: Event line '" << line << "' found without active date. Skipping." << std::endl;
                continue;
            }

            if (eventDescBuffer == "起床") {
                currentDayData.getupTime = formatTime(eventTimeBuffer);
            } else {
                currentDayData.rawEvents.push_back({eventTimeBuffer, eventDescBuffer});
            }
        } else {
            // Optional: std::cerr << "Warning: Skipping malformed line: '" << line << "'" << std::endl;
        }
    }

    if (!currentDayData.date.empty()) {
        processDayData(currentDayData, G_TEXT_MAPPING);
        writeDayData(outFile, currentDayData);
    }

    inFile.close();
    outFile.close(); 

    auto overallEndTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsedSeconds = overallEndTime - overallStartTime;

    std::cout << "Processing complete. Output written to " << outputFileName << std::endl;
    std::cout << "Time taken: " << std::fixed << std::setprecision(6) << elapsedSeconds.count() << " seconds." << std::endl;

    return 0;
}
