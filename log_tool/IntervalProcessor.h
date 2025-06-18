#ifndef INTERVAL_PROCESSOR_H
#define INTERVAL_PROCESSOR_H

#include <string>
#include <vector>
#include <unordered_map>
#include "json.hpp" // Assumes nlohmann/json.hpp is available

class IntervalProcessor {
public:
    // Constructor: Initializes the processor with the path to the configuration file.
    IntervalProcessor(const std::string& config_filename);

    // Processes the input file and writes the result to the output file.
    // Returns true on success, false on failure.
    bool processFile(const std::string& input_filepath, const std::string& output_filepath);

private:
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

        void clear();
    };

    // Configuration and state
    std::string config_filepath_;
    std::unordered_map<std::string, std::string> text_mapping_;

    // Private helper methods
    bool loadTextMapping();
    void processDayData(DayData& day);
    void writeDayData(std::ofstream& outFile, const DayData& day);
    std::string formatTime(const std::string& timeStrHHMM);
    bool isDateLine(const std::string& line);
    bool parseEventLine(const std::string& line, std::string& outTimeStr, std::string& outDescription);
};

#endif // INTERVAL_PROCESSOR_H