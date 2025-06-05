// This file utilizes functions defined in common_utils.h, specifically time_str_to_seconds.
// Do not redefine functions that are already present in common_utils.h within this file.

#ifndef DATA_PARSER_H
#define DATA_PARSER_H

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <regex>
#include <sqlite3.h>
#include <sstream>
#include <algorithm>
#include <map>
#include "common_utils.h" // 包含 time_str_to_seconds 的全局函数定义

// 函数声明
bool execute_sql_parser(sqlite3* db, const std::string& sql, const std::string& context_msg = "");

struct TimeRecordInternal {
    std::string date;
    std::string start;
    std::string end;
    std::string project_path;
    int duration_seconds;
};

class FileDataParser {
public:
    FileDataParser(const std::string& db_path);
    ~FileDataParser();
    bool is_db_open() const;
    bool parse_file(const std::string& filename);
    void commit_all();

private:
    sqlite3* db;
    std::string current_date;
    std::string current_status;
    std::string current_remark;
    std::string current_getup_time;
    std::vector<TimeRecordInternal> current_time_records_buffer;
    std::string current_file_name;
    bool current_date_processed;
    std::map<std::string, std::string> initial_top_level_parents;

    void _initialize_database();
    void _prepopulate_parent_child();
    void _handle_date_line(const std::string& line);
    void _handle_status_line(const std::string& line);
    void _handle_remark_line(const std::string& line);
    void _handle_getup_line(const std::string& line);
    void _handle_time_record_line(const std::string& line, int line_num);
    void _process_project_path(const std::string& project_path_orig);
    void _store_previous_date_data();
};

#endif // DATA_PARSER_H