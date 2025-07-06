#ifndef DATA_PARSER_H
#define DATA_PARSER_H

#include <string>
#include <vector>
#include <map>
#include <regex>
#include <sstream>
#include <nlohmann/json.hpp>

#include "common_utils.h"
#include "time_sheet_model.h" // MODIFIED: Include shared data structures

// --- DataFileParser Class Definition ---
class DataFileParser {
public:
    // Public member variables to hold the parsed data
    std::vector<DayData> days;
    std::vector<TimeRecordInternal> records;
    std::unordered_set<std::pair<std::string, std::string>, pair_hash> parent_child_pairs;

    /**
     * @brief DataFileParser constructor.
     * @param config_json A nlohmann::json object containing top-level parent mappings.
     */
    explicit DataFileParser(const nlohmann::json& config_json);

    ~DataFileParser();

    /**
     * @brief Parses the content of a single data file.
     * @param filename The path to the file to parse.
     * @return True if parsing was successful, false otherwise.
     */
    bool parse_file(const std::string& filename);

    /**
     * @brief Commits any remaining buffered data after all files are parsed.
     */
    void commit_all();

private:
    // Internal state for parsing
    std::string current_date;
    std::string current_status;
    std::string current_sleep;
    std::string current_remark;
    std::string current_getup_time;
    std::vector<TimeRecordInternal> buffered_records_for_day;
    std::string current_file_name;
    bool current_date_processed;
    std::map<std::string, std::string> initial_top_level_parents;
    const std::regex _time_record_regex;

    // Private helper methods for processing file content
    void _load_initial_parents(const nlohmann::json& config_json);
    void _process_lines(std::stringstream& buffer);
    void _process_single_line(const std::string& line, int line_num);
    void _handle_date_line(const std::string& line);
    void _handle_status_line(const std::string& line);
    void _handle_sleep_line(const std::string& line);
    void _handle_remark_line(const std::string& line);
    void _handle_getup_line(const std::string& line);
    void _handle_time_record_line(const std::string& line, int line_num);
    void _process_project_path(const std::string& project_path_orig);
    void _store_previous_date_data();
};

#endif // DATA_PARSER_H