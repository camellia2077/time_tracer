// --- START OF FILE parsing/data_parser.h ---

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
#include <nlohmann/json.hpp>

// --- 数据结构  ---
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
    std::string sleep;
    std::string remark;
    std::string getup_time;
};

struct pair_hash {
    template <class T1, class T2>
    std::size_t operator () (const std::pair<T1,T2> &p) const {
        auto h1 = std::hash<T1>{}(p.first);
        auto h2 = std::hash<T2>{}(p.second);
        return h1 ^ (h2 << 1);
    }
};

// --- DataFileParser 类定义 ---
class DataFileParser {
public:
    std::vector<DayData> days;
    std::vector<TimeRecordInternal> records;
    std::unordered_set<std::pair<std::string, std::string>, pair_hash> parent_child_pairs;

    /**
     * @brief DataFileParser 的构造函数。
     * @details 现在直接接收一个json对象来初始化父级映射。
     * @param config_json 包含顶层父级映射的 nlohmann::json 对象。
     */
    explicit DataFileParser(const nlohmann::json& config_json);

    ~DataFileParser();
    bool parse_file(const std::string& filename);
    void commit_all();

private:
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

    /**
     * @brief 【修改】从传入的json对象加载初始的父级项目映射。
     */
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