#include "data_parser.h"
#include <fstream>
#include <iostream>
#include <algorithm>
#include <stdexcept>
// 调用 common_utils.h 中的 time_str_to_seconds 函数
// --- DataFileParser Constructor & Destructor ---

DataFileParser::DataFileParser() // 构造函数与析构函数
    : current_date_processed(false), // 初始化 current_date_processed 为 false
    _time_record_regex(R"((\d{2}:\d{2})~(\d{2}:\d{2})(.+))")// WARNING: Do not change this regex — it matches the expected input format 用于匹配 "HH:MM~HH:MMevent" 格式的时间记录行
{
    initial_top_level_parents = // 映射，包含预定义的顶级父子关系，如 "study" 对应 "study"，"code" 对应 "code"。
    {
        {"study", "mystudy"}
    };
}

DataFileParser::~DataFileParser() {
    // The commit_all() method should be called explicitly by the owner
    // to ensure any lingering data from the last file is processed.
    //析构函数，没有特定的清理逻辑，因为 commit_all() 应由外部显式调用
}

// --- Public Member Functions ---

void DataFileParser::commit_all() {
    // Store any lingering data from the last date in the last file processed.
    _store_previous_date_data(); // 调用 _store_previous_date_data() 方法，确保文件中最后一个日期的数据也被存储起来
}

// 将原始的 parse_file 拆分为更小、更专注的函数
bool DataFileParser::parse_file(const std::string& filename) { // 解析文件的核心公共方法
    std::ifstream file(filename); // 创建一个输入文件流对象并尝试打开指定文件
    if (!file.is_open()) { // 如果文件打开失败
        std::cerr << "Error: Cannot open file " << filename << std::endl; // 向标准错误流 std::cerr 输出错误信息
        return false; // 返回 false 表示失败
    }

    std::stringstream buffer; // 创建一个字符串流
    buffer << file.rdbuf(); // 为了高效处理，将整个文件内容一次性读入一个字符串流 buffer 中
    file.close(); // 关闭文件
    current_file_name = filename; // 保存当前文件名，主要用于在发生错误时提供上下文信息。
    
    bool success = true; // 初始化成功状态为 true
    try { // 使用 try-catch 块来捕获解析过程中可能发生的任何标准异常，确保程序的健壮性。
        _process_lines(buffer); // 调用私有方法处理所有行
    } catch (const std::exception& e) { // 捕获标准异常
        std::cerr << current_file_name << ": An error occurred during parsing: " << e.what() << std::endl; // 输出错误信息
        success = false; // 设置成功状态为 false
    }

    return success; // 返回最终的成功状态
}


// --- Private Member Functions ---

void DataFileParser::_process_lines(std::stringstream& buffer) { // 逐行处理文件内容流
    std::string line; // 用于存储每一行的内容
    int line_num = 0; // 初始化行号计数器
    
    while (std::getline(buffer, line)) { // 逐行读取字符串流 buffer 中的内容
        line_num++; // 行号递增
        _process_single_line(line, line_num); // 调用单个行处理函数
    }
}

void DataFileParser::_process_single_line(const std::string& line, int line_num) { // 处理单行内容
    auto trimmed_line = line; // 复制行内容以便修改
    trimmed_line.erase(0, trimmed_line.find_first_not_of(" \t\n\r\f\v")); // 删除行首的所有空白字符
    trimmed_line.erase(trimmed_line.find_last_not_of(" \t\n\r\f\v") + 1); // 删除行尾的所有空白字符

    if (trimmed_line.empty()) return; // 如果修剪后是空行，则忽略

    // 根据行的前缀分发到不同的处理函数
    if (trimmed_line.rfind("Date:", 0) == 0) { // 如果行以 "Date:" 开头
        _store_previous_date_data();  // 当遇到一个新的 "Date:" 行时，意味着前一个日期的数据已经完整，此时需要先将前一天缓存的所有数据进行存储。
        _handle_date_line(trimmed_line);
    } else if (trimmed_line.rfind("Status:", 0) == 0) { // 如果行以 "Status:" 开头
        _handle_status_line(trimmed_line);
    } else if (trimmed_line.rfind("Sleep:", 0) == 0) { // 新增：处理Sleep行
        _handle_sleep_line(trimmed_line);
    } else if (trimmed_line.rfind("Remark:", 0) == 0) { // 如果行以 "Remark:" 开头
        _handle_remark_line(trimmed_line);
    } else if (trimmed_line.rfind("Getup:", 0) == 0) { // 如果行以 "Getup:" 开头
        _handle_getup_line(trimmed_line);
    } else if (trimmed_line.find('~') != std::string::npos) { // 如果行包含 "~" 字符，认为是时间记录行
        _handle_time_record_line(trimmed_line, line_num);
    }
}


void DataFileParser::_handle_date_line(const std::string& line) {
    if (line.length() > 5) { // 确保行长度足够提取日期
        current_date = line.substr(5); // 提取 "Date:" 后面的子字符串作为日期
        current_date.erase(0, current_date.find_first_not_of(" \t")); // 删除日期字符串前面的多余空格
        current_date.erase(current_date.find_last_not_of(" \t") + 1); // 删除日期字符串后面的多余空格

        // Reset for the new day
        // 为新的一天重置所有状态
        current_status = "False"; // 重置状态为 "False"
        current_sleep = "False";  // 新增：重置Sleep状态为 "False"
        current_remark = ""; // 重置备注为空字符串
        current_getup_time = "00:00"; // 重置起床时间为 "00:00"
        buffered_records_for_day.clear(); // 清空当天的时间记录缓冲区
        current_date_processed = false; // 标记当前日期为未处理
    }
}

void DataFileParser::_handle_status_line(const std::string& line) {
    if (line.length() > 7) { // 确保行长度足够提取状态
        current_status = line.substr(7); // 提取 "Status:" 后面的子字符串作为状态
    }
}

// 新增方法
void DataFileParser::_handle_sleep_line(const std::string& line) {
    if (line.length() > 6) { // "Sleep:" is 6 chars
        current_sleep = line.substr(6); // 提取 "Sleep:" 后面的子字符串作为状态
    }
}

void DataFileParser::_handle_remark_line(const std::string& line) {
    if (line.length() > 7) { // 确保行长度足够提取备注
        current_remark = line.substr(7); // 提取 "Remark:" 后面的子字符串作为备注
    }
}

void DataFileParser::_handle_getup_line(const std::string& line) {
    if (line.length() > 6) { // 确保行长度足够提取起床时间
        current_getup_time = line.substr(6); // 提取 "Getup:" 后面的子字符串作为起床时间
    }
}

void DataFileParser::_handle_time_record_line(const std::string& line, int line_num) {
    std::smatch matches; // 用于存储正则表达式的匹配结果
    // 检查行是否与预定义的时间记录正则表达式匹配，并且有4个捕获组（整个匹配，开始时间，结束时间，项目路径）
    if (std::regex_match(line, matches, _time_record_regex) && matches.size() == 4) 
    { 
        std::string start_time_str = matches[1].str(); // 提取开始时间字符串
        std::string end_time_str = matches[2].str(); // 提取结束时间字符串
        std::string project_path = matches[3].str(); // 提取项目路径字符串

        int start_seconds = time_str_to_seconds(start_time_str); // 调用 common_utils.h 中的 time_str_to_seconds 函数，将开始时间转换为秒
        int end_seconds = time_str_to_seconds(end_time_str); // 调用 common_utils.h 中的 time_str_to_seconds 函数，将结束时间转换为秒
        // 计算持续时间（秒），处理跨天的情况（结束时间小于开始时间）
        int duration_seconds = (end_seconds < start_seconds) ? ((end_seconds + 24 * 3600) - start_seconds) : (end_seconds - start_seconds);

        // 将解析出的时间记录存入当日缓冲区
        buffered_records_for_day.push_back({current_date, start_time_str, end_time_str, project_path, duration_seconds});
        _process_project_path(project_path); // 处理项目路径以建立父子关系
    }
}

void DataFileParser::_process_project_path(const std::string& project_path_orig) {
    // 调用 common_utils.h 中的 split_string 函数，按 '_' 分隔符切分项目路径。
    std::vector<std::string> segments = split_string(project_path_orig, '_');

    if (segments.empty()) {
        return; // 如果没有分段，则直接返回
    }

    // 将预定义的顶级父子关系添加到总的父子关系集合中
    // 每次调用此函数时都会运行此循环，但由于 parent_child_pairs 是一个集合，
    // 重复插入不会产生副作用。
    for (const auto& pair : initial_top_level_parents) {
        parent_child_pairs.insert({pair.first, pair.second});
    }

    // 仅当路径包含多个段时才创建层级关系。
    // 例如，对于 "a_b_c"，将创建 (a_b, a) 和 (a_b_c, a_b) 的关系。
    // 对于只有一个段的路径（例如 "study" 或 "workout"），只有当它存在于
    // initial_top_level_parents 中时才会创建关系（已在上面的循环中处理）。
    // 此实现避免了为不在映射中的顶级项目（如 "workout"）创建自我父关系（例如 {workout, workout}）。
    if (segments.size() > 1) {
        std::string parent_path = segments[0];
        for (size_t i = 1; i < segments.size(); ++i) {
            std::string child_path = parent_path + "_" + segments[i];
            parent_child_pairs.insert({child_path, parent_path});
            parent_path = child_path; // 对于下一个段，当前子路径成为父路径
        }
    }
}

void DataFileParser::_store_previous_date_data() {
    // 如果当前日期为空（文件开始）或当前日期数据已被处理，则直接返回
    if (current_date.empty() || current_date_processed) return;

    // Add the collected day info to the main 'days' vector
    // 将收集到的当天信息添加到 'days' 向量中
    // 修改：在构造DayData时加入current_sleep
    days.push_back({current_date, current_status, current_sleep, current_remark, current_getup_time});

    // Add all buffered time records for that day to the main 'records' vector
    // 将缓冲区中所有当天的时间记录添加到 'records' 主向量中
    records.insert(records.end(), buffered_records_for_day.begin(), buffered_records_for_day.end());

    buffered_records_for_day.clear(); // 清空当日记录缓冲区，为下一天做准备
    current_date_processed = true; // 标记当前日期的数据已处理完毕
}
