#ifndef DATA_PARSER_H
#define DATA_PARSER_H

#include <string>
#include <vector>
#include <map>
#include <unordered_set>
#include <utility>
#include <regex>
#include <sstream>

#include "common_utils.h"// 调用 common_utils.h 中的 time_str_to_seconds 函数

// --- Data Structures for In-Memory Storage ---

struct TimeRecordInternal { 
    std::string date; // TimeRecordInternal 中的 date 和 DayData 中的 date 将会用于将所有相关信息关联到同一个日期,例如20241031
    std::string start; // 时间段的开始时间,例如09:00
    std::string end; // 时间段的结束时间,例如10:00
    std::string project_path; // 相关的项目名称 sleep_night
    int duration_seconds; // 项目持续的时间
};

struct DayData {// 表示某一天的总体数据，不包括具体的时间记录
    std::string date; // TimeRecordInternal 中的 date 和 DayData 中的 date 将会用于将所有相关信息关联到同一个日期
    std::string status; // 状态 False或者True
    std::string remark; //备注内容,可与为任何字符
    std::string getup_time; // 起床时间,例如Getup:09:10
};

struct pair_hash {// 为 std::pair<std::string, std::string> 提供自定义哈希函数，以便在 std::unordered_set 中使用
    template <class T1, class T2>
    std::size_t operator () (const std::pair<T1,T2> &p) const {// operator()：实现了简单的哈希组合，将两个字符串的哈希值进行异或和位移操作
        auto h1 = std::hash<T1>{}(p.first); // 计算第一个元素的哈希值
        auto h2 = std::hash<T2>{}(p.second); // 计算第二个元素的哈希值
        return h1 ^ (h2 << 1); // 组合两个哈希值
    }
};

class DataFileParser {//负责解析输入文件并填充上述数据结构
public:
    // --- Public Data Stores ---公共数据存储
    // These are populated by the parser and then read by the importer.
    std::vector<DayData> days;// 存储从文件中解析出的所有 DayData 对象
    std::vector<TimeRecordInternal> records;// 存储从文件中解析出的所有 TimeRecordInternal 对象。
    std::unordered_set<std::pair<std::string, std::string>, pair_hash> parent_child_pairs;//存储解析出的项目路径中的父子关系对（例如 {"study_math", "STUDY"}）。

    // --- Constructor & Public Methods ---构造函数与公共方法
    DataFileParser(); // 初始化内部状态,包括时间记录的正则表达式 _time_record_regex 和初始的顶级父类映射 initial_top_level_parents
    ~DataFileParser(); // 析构函数
    bool parse_file(const std::string& filename); // 解析指定的文件。如果文件成功打开并解析，返回 true；否则返回 false
    void commit_all(); // 处理并存储最后一个日期的数据。在所有文件解析完成后，应显式调用此方法，以确保所有残留数据都被处理

private:
    // --- Internal State for Parsing ---
    std::string current_date; // 当前正在处理的日期字符串
    std::string current_status; // 当前日期的状态
    std::string current_remark; // 当前日期的备注
    std::string current_getup_time; // 当前日期的起床时间
    std::vector<TimeRecordInternal> buffered_records_for_day; // 缓冲区，用于存储当前日期下的所有时间记录，直到遇到新的日期行才将其提交到 records
    std::string current_file_name; // 当前正在解析的文件名，用于错误报告
    bool current_date_processed; // 标志位，指示当前日期的数据是否已处理并存储
    std::map<std::string, std::string> initial_top_level_parents;// 预定义的顶级项目及其对应的父级映射（例如 {"study", "STUDY"}, {"code", "CODE"}）
    const std::regex _time_record_regex; // 用于匹配时间记录行的正则表达式。它匹配 "HH:MM~HH:MMevent" 格式。

    // --- Private Helper Methods ---
    void _process_lines(std::stringstream& buffer); // 逐行处理文件内容流
    void _process_single_line(const std::string& line, int line_num); // 处理单行内容，并根据内容分发到相应的处理函数
    void _handle_date_line(const std::string& line); // 处理 "Date:" 开头的行，更新 current_date 并重置当日数据缓冲区
    void _handle_status_line(const std::string& line); // 处理 "Status:" 开头的行，更新 current_status
    void _handle_remark_line(const std::string& line); // 处理 "Remark:" 开头的行，更新 current_remark
    void _handle_getup_line(const std::string& line); // 处理 "Getup:" 开头的行，更新 current_getup_time
    void _handle_time_record_line(const std::string& line, int line_num); // 处理包含 "~" 的时间记录行。使用正则表达式提取开始时间、结束时间和项目路径，计算持续时间，并将记录添加到 buffered_records_for_day
    void _process_project_path(const std::string& project_path_orig); // 处理项目路径，将其中的空格替换为下划线，并分析路径中的分段以建立父子关系，填充 parent_child_pairs。此方法会调用 common_utils.h 中的 split_string 函数
    void _store_previous_date_data(); // 将 buffered_records_for_day 和当前日期的 DayData 提交到 records 和 days 向量中。在遇到新的 "Date:" 行时或调用 commit_all() 时触发
};

#endif // DATA_PARSER_H