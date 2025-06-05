#ifndef DATA_PARSER_H
#define DATA_PARSER_H

#include <string>
#include <vector>
#include <map>
#include <unordered_set>
#include <utility>
#include <regex>
#include <sstream>

#include "common_utils.h"

// --- Data Structures for In-Memory Storage ---

struct TimeRecordInternal {
    std::string date;
    std::string start;
    std::string end;
    std::string project_path;
    int duration_seconds;
};

struct DayData {
    std::string date;
    std::string status;
    std::string remark;
    std::string getup_time;
};

// Custom hash function for std::pair, needed for the unordered_set
struct pair_hash {
    template <class T1, class T2>
    std::size_t operator () (const std::pair<T1,T2> &p) const {
        auto h1 = std::hash<T1>{}(p.first);
        auto h2 = std::hash<T2>{}(p.second);
        // A simple way to combine hashes
        return h1 ^ (h2 << 1);
    }
};

class DataFileParser {
public:
    // --- Public Data Stores ---
    // These are populated by the parser and then read by the importer.
    std::vector<DayData> days;
    std::vector<TimeRecordInternal> records;
    std::unordered_set<std::pair<std::string, std::string>, pair_hash> parent_child_pairs;

    // --- Constructor & Public Methods ---
    DataFileParser();
    ~DataFileParser();
    bool parse_file(const std::string& filename);
    void commit_all();

private:
    // --- Internal State for Parsing ---
    std::string current_date;
    std::string current_status;
    std::string current_remark;
    std::string current_getup_time;
    std::vector<TimeRecordInternal> buffered_records_for_day;
    std::string current_file_name;
    bool current_date_processed;
    std::map<std::string, std::string> initial_top_level_parents;

    // --- Private Helper Methods ---
    void _handle_date_line(const std::string& line);
    void _handle_status_line(const std::string& line);
    void _handle_remark_line(const std::string& line);
    void _handle_getup_line(const std::string& line);
    void _handle_time_record_line(const std::string& line, int line_num);
    void _process_project_path(const std::string& project_path_orig);
    void _store_previous_date_data(); // Replaces the file-writing method
};

#endif // DATA_PARSER_H