#include "data_parser.h"
#include <fstream>
#include <iostream>
#include <algorithm>
#include <stdexcept>
// 调用 common_utils.h 中的 time_str_to_seconds 函数
// --- DataFileParser Constructor & Destructor ---

DataFileParser::DataFileParser() 
    : current_date_processed(false),
    _time_record_regex(R"((\d{2}:\d{2})~(\d{2}:\d{2})\s*(.+))")// WARNING: Do not change this regex — it matches the expected input format
{
    initial_top_level_parents = 
    {
        {"study", "STUDY"},
        {"code", "CODE"}
    };
}

DataFileParser::~DataFileParser() {
    // The commit_all() method should be called explicitly by the owner
    // to ensure any lingering data from the last file is processed.
}

// --- Public Member Functions ---

void DataFileParser::commit_all() {
    // Store any lingering data from the last date in the last file processed.
    _store_previous_date_data();
}

bool DataFileParser::parse_file(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Cannot open file " << filename << std::endl;
        return false;
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    file.close();
    current_file_name = filename;
    bool success = true;
    try {
        std::string line;
        int line_num = 0;
        
        while (std::getline(buffer, line)) { 
            line_num++;
            line.erase(0, line.find_first_not_of(" \t\n\r\f\v"));
            line.erase(line.find_last_not_of(" \t\n\r\f\v") + 1);
            if (line.empty()) continue;
            if (line.rfind("Date:", 0) == 0) {
                _store_previous_date_data(); 
                _handle_date_line(line);
            } else if (line.rfind("Status:", 0) == 0) {
                _handle_status_line(line);
            } else if (line.rfind("Remark:", 0) == 0) {
                _handle_remark_line(line);
            } else if (line.rfind("Getup:", 0) == 0) {
                _handle_getup_line(line);
            } else if (line.find('~') != std::string::npos) {
                _handle_time_record_line(line, line_num);
            }
        }
    } catch (const std::exception& e) {
        std::cerr << current_file_name << ": An error occurred during parsing: " << e.what() << std::endl;
        success = false;
    }

    return success;
}

// --- Private Member Functions ---

void DataFileParser::_handle_date_line(const std::string& line) {
    if (line.length() > 5) {
        current_date = line.substr(5);
        current_date.erase(0, current_date.find_first_not_of(" \t"));
        current_date.erase(current_date.find_last_not_of(" \t") + 1);

        // Reset for the new day
        current_status = "False";
        current_remark = "";
        current_getup_time = "00:00";
        buffered_records_for_day.clear();
        current_date_processed = false;
    }
}

void DataFileParser::_handle_status_line(const std::string& line) {
    if (line.length() > 7) current_status = line.substr(7);
}

void DataFileParser::_handle_remark_line(const std::string& line) {
    if (line.length() > 7) current_remark = line.substr(7);
}

void DataFileParser::_handle_getup_line(const std::string& line) {
    if (line.length() > 6) current_getup_time = line.substr(6);
}

void DataFileParser::_handle_time_record_line(const std::string& line, int line_num) {
    std::smatch matches;
    if (std::regex_match(line, matches, _time_record_regex) && matches.size() == 4) 
    { 
        std::string start_time_str = matches[1].str();
        std::string end_time_str = matches[2].str();
        std::string project_path = matches[3].str();

        int start_seconds = time_str_to_seconds(start_time_str);// 调用 common_utils.h 中的 time_str_to_seconds 函数
        int end_seconds = time_str_to_seconds(end_time_str);
        int duration_seconds = (end_seconds < start_seconds) ? ((end_seconds + 24 * 3600) - start_seconds) : (end_seconds - start_seconds);

        buffered_records_for_day.push_back({current_date, start_time_str, end_time_str, project_path, duration_seconds});
        _process_project_path(project_path);
    }
}

void DataFileParser::_process_project_path(const std::string& project_path_orig) {
    std::string project_path = project_path_orig;
    std::replace(project_path.begin(), project_path.end(), ' ', '_');
    std::stringstream ss(project_path);
    std::string segment;
    std::vector<std::string> segments;
    while(std::getline(ss, segment, '_')) {
        if (!segment.empty()) segments.push_back(segment);
    }

    if (segments.empty()) return;
    
    for (const auto& pair : initial_top_level_parents) {
        parent_child_pairs.insert({pair.first, pair.second});
    }

    std::string current_full_path = "";
    std::string parent_of_current_segment;
    for (size_t i = 0; i < segments.size(); ++i) {
        if (i == 0) {
            current_full_path = segments[i];
            auto it = initial_top_level_parents.find(current_full_path);
            if (it != initial_top_level_parents.end()) {
                parent_of_current_segment = it->second;
            } else {
                parent_of_current_segment = current_full_path;
                std::transform(parent_of_current_segment.begin(), parent_of_current_segment.end(), parent_of_current_segment.begin(), ::toupper);
            }
        } else {
            parent_of_current_segment = current_full_path;
            current_full_path += "_" + segments[i];
        }
        parent_child_pairs.insert({current_full_path, parent_of_current_segment});
    }
}

void DataFileParser::_store_previous_date_data() {
    if (current_date.empty() || current_date_processed) return;

    // Add the collected day info to the main 'days' vector
    days.push_back({current_date, current_status, current_remark, current_getup_time});

    // Add all buffered time records for that day to the main 'records' vector
    records.insert(records.end(), buffered_records_for_day.begin(), buffered_records_for_day.end());

    buffered_records_for_day.clear();
    current_date_processed = true;
}