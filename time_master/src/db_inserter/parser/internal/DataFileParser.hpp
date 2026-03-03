#ifndef DATA_PARSER_HPP
#define DATA_PARSER_HPP

#include <string>
#include <vector>
#include <map>
#include <regex>
#include <sstream>

#include "common/common_utils.hpp"
#include "db_inserter/model/time_sheet_model.hpp"
#include "ParserConfig.hpp" 

class DataFileParser {
public:
    std::vector<DayData> days;
    std::vector<TimeRecordInternal> records;
    std::unordered_set<std::pair<std::string, std::string>, pair_hash> parent_child_pairs;

    explicit DataFileParser(const ParserConfig& config);

    ~DataFileParser();
    bool parse_file(const std::string& filename);
    void commit_all();

private:
    std::string current_date;
    int current_year;   // [新增] 用于暂存当前日期的年份
    int current_month;  // [新增] 用于暂存当前日期的月份
    std::string current_status;
    std::string current_sleep;
    std::string current_remark;
    std::string current_getup_time;
    std::vector<TimeRecordInternal> buffered_records_for_day;
    std::string current_file_name;
    bool current_date_processed;
    std::map<std::string, std::string> initial_top_level_parents;
    const std::regex _time_record_regex;

    void _process_lines(std::stringstream& buffer);
    void _process_single_line(const std::string& line);
    void _handle_date_line(const std::string& line);
    void _handle_status_line(const std::string& line);
    void _handle_sleep_line(const std::string& line);
    void _handle_remark_line(const std::string& line);
    void _handle_getup_line(const std::string& line);
    void _handle_time_record_line(const std::string& line);
    void _process_project_path(const std::string& project_path_orig);
    void _store_previous_date_data();
};

#endif // DATA_PARSER_HPP
