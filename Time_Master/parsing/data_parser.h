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
#include <nlohmann/json.hpp>

// --- 内存中存储的数据结构 ---

/**
 * @brief 存储单条时间记录的内部表示。
 * @details 从文本文件中解析出的每一行时间记录（如 "09:00~10:00 project_a"）都会被转换成这个结构体。
 */
struct TimeRecordInternal {
    std::string date;           // 记录所属的日期，例如："20241031"
    std::string start;          // 时间段的开始时间，例如："09:00"
    std::string end;            // 时间段的结束时间，例如："10:00"
    std::string project_path;   // 相关联的项目名称，例如："sleep_night"
    int duration_seconds;       // 项目的持续时间，以秒为单位
};

/**
 * @brief 存储某一天的总体摘要数据。
 * @details 不包括具体的时间分段记录，只包含当天的状态、备注等信息。
 */
struct DayData {
    std::string date;           // 日期，用于与其他数据关联
    std::string status;         // 当天状态，例如："True" 或 "False"
    std::string sleep;          // 新增：睡眠状态，例如："True" 或 "False"
    std::string remark;         // 备注内容
    std::string getup_time;     // 起床时间，例如："09:10"
};

/**
 * @brief 为 std::pair<std::string, std::string> 提供自定义哈希函数。
 * @details 这是在 std::unordered_set 中使用 std::pair 作为键所必需的，因为标准库没有为 std::pair 提供默认的哈希实现。
 */
struct pair_hash {
    /**
     * @brief 哈希运算符重载。
     * @details 实现了一个简单的哈希组合算法，对两个字符串的哈希值执行按位异或和移位操作，以生成一个组合哈希值。
     * @param p 要计算哈希值的 std::pair 对象。
     * @return 计算出的组合哈希值。
     */
    template <class T1, class T2>
    std::size_t operator () (const std::pair<T1,T2> &p) const {
        auto h1 = std::hash<T1>{}(p.first); // 计算第一个元素的哈希值
        auto h2 = std::hash<T2>{}(p.second); // 计算第二个元素的哈希值
        return h1 ^ (h2 << 1); // 结合两个哈希值
    }
};

/**
 * @class DataFileParser
 * @brief 负责解析特定格式的文本文件，并将解析出的数据填充到内部定义的数据结构中。
 * @details 这个类是数据处理流水线的起点，它读取原始文本日志，并将其转换为程序可以理解和处理的结构化数据。
 */
class DataFileParser {
public:
    // --- 公共数据存储 ---
    // 这些数据由解析器填充，然后由其他模块（如数据库插入器）读取。
    std::vector<DayData> days;              // 存储从文件中解析出的所有 DayData 对象
    std::vector<TimeRecordInternal> records;// 存储从文件中解析出的所有 TimeRecordInternal 对象。
    std::unordered_set<std::pair<std::string, std::string>, pair_hash> parent_child_pairs;// 存储从项目路径中解析出的父子关系对（例如：{"study_math", "STUDY"}）。

    // --- 构造函数与公共方法 ---

    /**
     * @brief DataFileParser 的构造函数。
     * @details 初始化内部状态，最重要的是，它会调用 `_load_initial_parents` 方法从指定的JSON配置文件中加载顶层项目的父级映射。
     * @param config_path 指向包含顶层父级映射的JSON配置文件的路径。默认为 "config.json"。
     */
    DataFileParser(const std::string& config_path = "config.json");

    /**
     * @brief DataFileParser 的析构函数。
     */
    ~DataFileParser();

    /**
     * @brief 解析指定的文本文件。
     * @details 这是该类的主要入口点。它读取整个文件内容到内存，然后逐行处理。使用 try-catch 块来确保解析过程的健壮性。
     * @param filename 要解析的文件的完整路径。
     * @return 如果文件成功打开并解析，则返回 true；否则返回 false。
     */
    bool parse_file(const std::string& filename);

    /**
     * @brief 提交所有剩余的缓冲数据。
     * @details 在解析完所有文件后，应显式调用此方法。因为数据通常是在遇到下一个"Date:"行时才提交的，所以最后一个日期的数据需要通过此方法手动提交。
     */
    void commit_all();

private:
    // --- 用于解析的内部状态 ---
    std::string current_date;               // 当前正在处理的日期字符串
    std::string current_status;             // 当前日期的状态
    std::string current_sleep;              // 新增：当前日期的睡眠状态
    std::string current_remark;             // 当前日期的备注
    std::string current_getup_time;         // 当前日期的起床时间
    std::vector<TimeRecordInternal> buffered_records_for_day; // 一个缓冲区，用于临时存储当前日期的所有时间记录。
    std::string current_file_name;          // 当前正在解析的文件名，主要用于错误报告。
    bool current_date_processed;            // 一个标志，指示当前日期的数据是否已被处理和存储，以防止重复提交。
    std::map<std::string, std::string> initial_top_level_parents;// 从 JSON 加载的预定义顶层项目及其对应的父级映射。
    const std::regex _time_record_regex;    // 用于匹配 "HH:MM~HH:MM事件" 格式时间记录行的正则表达式。

    // --- 私有辅助方法 ---

    /**
     * @brief 从 JSON 文件加载初始的父级项目映射。
     * @details 此方法在构造函数中被调用，用于填充 `initial_top_level_parents` 映射。
     * @param config_path JSON 配置文件的路径。
     */
    void _load_initial_parents(const std::string& config_path);

    /**
     * @brief 逐行处理已加载到内存中的文件内容。
     * @param buffer 包含整个文件内容的字符串流。
     */
    void _process_lines(std::stringstream& buffer);

    /**
     * @brief 处理单行文本内容。
     * @details 这是一个调度函数，它会修剪行首尾的空白，并根据行的前缀（如"Date:", "Status:"）将其分派给相应的处理函数。
     * @param line 要处理的单行字符串。
     * @param line_num 当前行的行号，用于错误报告。
     */
    void _process_single_line(const std::string& line, int line_num);

    /**
     * @brief 处理以 "Date:" 开头的行。
     * @details 当遇到新的日期行时，首先会调用 `_store_previous_date_data` 来保存前一天的数据，然后更新当前日期并重置所有每日状态变量。
     * @param line 包含日期信息的行。
     */
    void _handle_date_line(const std::string& line);

    /**
     * @brief 处理以 "Status:" 开头的行，更新 `current_status` 状态。
     * @param line 包含状态信息的行。
     */
    void _handle_status_line(const std::string& line);

    /**
     * @brief 新增：处理以 "Sleep:" 开头的行，更新 `current_sleep` 状态。
     * @param line 包含睡眠状态信息的行。
     */
    void _handle_sleep_line(const std::string& line);

    /**
     * @brief 处理以 "Remark:" 开头的行，更新 `current_remark` 状态。
     * @param line 包含备注信息的行。
     */
    void _handle_remark_line(const std::string& line);

    /**
     * @brief 处理以 "Getup:" 开头的行，更新 `current_getup_time` 状态。
     * @param line 包含起床时间信息的行。
     */
    void _handle_getup_line(const std::string& line);

    /**
     * @brief 处理时间记录行（包含"~"的行）。
     * @details 使用正则表达式从行中提取开始时间、结束时间和项目路径，计算持续时间，然后将结果存入 `buffered_records_for_day` 缓冲区。
     * @param line 包含时间记录的行。
     * @param line_num 当前行号，用于可能的错误报告。
     */
    void _handle_time_record_line(const std::string& line, int line_num);

    /**
     * @brief 处理项目路径字符串，以建立父子层级关系。
     * @details 例如，对于路径 "a_b_c"，此方法会建立 ("a_b", "a") 和 ("a_b_c", "a_b") 这样的父子关系对，并存入 `parent_child_pairs` 集合。
     * @param project_path_orig 从时间记录行中提取的原始项目路径。
     */
    void _process_project_path(const std::string& project_path_orig);

    /**
     * @brief 将当前日期缓冲的所有数据（日摘要和时间记录）提交到最终的存储向量中。
     * @details 此方法在遇到新的"Date:"行或调用`commit_all()`时被触发。
     */
    void _store_previous_date_data();
};

#endif // DATA_PARSER_H