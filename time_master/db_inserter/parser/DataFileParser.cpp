#include "DataFileParser.h"
#include <fstream>
#include <iostream>
#include <algorithm>
#include <stdexcept>


// --- DataFileParser Constructor & Destructor ---

DataFileParser::DataFileParser(const ParserConfig& config)
    // 成员初始化列表
    : 
    current_date_processed(false),
    _time_record_regex(R"(([0-9]{2}:[0-9]{2})~([0-9]{2}:[0-9]{2})(.+))")// 用于捕获 09:00~10:00Study_code
    
{
    initial_top_level_parents = config.initial_top_level_parents; // 传入父项目的映射
}

DataFileParser::~DataFileParser() {
    // Destructor is empty, commit_all() is called externally.
}
// --- Public Member Functions ---
bool DataFileParser::parse_file(const std::string& filename) { 
    std::ifstream file(filename); // 创建file对象，用于传入filename指定的文件
    if (!file.is_open()) {
        std::cerr << RED_COLOR << "Error: " << RESET_COLOR << "Cannot open file " << filename << std::endl;
        return false;
    }
    std::stringstream buffer; // 创建一个 std::stringstream 对象 buffer
    buffer << file.rdbuf(); // file.rdbuf() 返回与 file 对象关联的文件缓冲区指针
    file.close(); // 一旦文件内容被读取到 buffer 中，就关闭文件,释放内存资源
    current_file_name = filename;
    
    bool success = true;
    try {
        _process_lines(buffer);
    } catch (const std::exception& e) {
        std::cerr << current_file_name << ": An error occurred during parsing: " << e.what() << std::endl;
        success = false;
    }
    return success;
}

void DataFileParser::commit_all() { 
    _store_previous_date_data();
}


// --- Private Member Functions ---

//逐行处理文件内容缓冲区中的所有行
void DataFileParser::_process_lines(std::stringstream& buffer) { 
    std::string line;
    int line_num = 0;
    while (std::getline(buffer, line)) //从buffer中读取直到遇到换行符或文件末尾，并将读取的内容存储到 line 变量中
    {
        line_num++;
        _process_single_line(line);
    }
}
// 处理单行内容
void DataFileParser::_process_single_line(const std::string& line) { 
    // 1. 去除首尾多余
    auto trimmed_line = line; // 创建line内容的副本，在函数内部对字符串进行修改,不对实际内容进行修改
    // 去除字符串开头的所有空白字符 返回找到的第一个非空白字符的索引 如果字符串是 "   Hello"，它会返回 3（'H' 的索引）
    trimmed_line.erase(0, trimmed_line.find_first_not_of(" \t\n\r\f\v")); 

    // ind_last_not_of() 返回的是最后一个非空白字符的索引。为了删除这些非空白字符之后的所有空白字符，我们需要从这个索引的下一个位置开始删除
    // erase 从 索引 位置开始，删除字符串中从 索引 到字符串末尾的所有字符。
    trimmed_line.erase(trimmed_line.find_last_not_of(" \t\n\r\f\v") + 1);

    // 2. 空行检查
    // 如果去除首尾多余字符后trimmed_line为空，说明这一行没有字符串，直接return不做处理
    if (trimmed_line.empty()) return;

    // 3. 内容类型判断与分派

    // C++20 standard library provides string::starts_with.
    if (trimmed_line.starts_with("Date:")) {
        _store_previous_date_data();
        _handle_date_line(trimmed_line);
    } else if (trimmed_line.starts_with("Status:")) {
        _handle_status_line(trimmed_line);
    } else if (trimmed_line.starts_with("Sleep:")) {
        _handle_sleep_line(trimmed_line);
    } else if (trimmed_line.starts_with("Remark:")) {
        _handle_remark_line(trimmed_line);
    } else if (trimmed_line.starts_with("Getup:")) {
        _handle_getup_line(trimmed_line);
    } else if (trimmed_line.find('~') != std::string::npos) { //不用starts_with是因为'~'在文本的中间
        _handle_time_record_line(trimmed_line);
    }
}

void DataFileParser::_handle_date_line(const std::string& line) { 

        current_date = line.substr(5); // 截取标题行内容例如，Date:20030501五个字符后的中的日期，例如20030501
        current_status = "False"; // Status:默认为False
        current_sleep = "False"; // Sleep:默认为False
        current_remark = ""; // Remark:默认为空
        current_getup_time = "00:00"; //Getup:默认为00:00
        buffered_records_for_day.clear();
        current_date_processed = false;
}

void DataFileParser::_handle_status_line(const std::string& line) { // "Status:" is 7 chars
    current_status = line.substr(7);
}

void DataFileParser::_handle_sleep_line(const std::string& line) { // "Sleep:" is 6 chars 
    current_sleep = line.substr(6);
}

void DataFileParser::_handle_remark_line(const std::string& line) { // "Remark:" is 7 chars 
    current_remark = line.substr(7);
}

void DataFileParser::_handle_getup_line(const std::string& line) { // "Getup:" is 6 chars 
    current_getup_time = line.substr(6);
}

void DataFileParser::_handle_time_record_line(const std::string& line) {
    std::smatch matches;

    // Directly perform the match to populate the 'matches' object.
    // WARNING: This assumes the 'line' will ALWAYS be valid.
    // If 'line' does not match the regex, subsequent access to matches[1], etc.,
    // will cause the program to crash.
    std::regex_match(line, matches, _time_record_regex);

    // Proceed to extract data without validation.
    std::string start_time_str = matches[1].str();
    std::string end_time_str = matches[2].str();
    std::string project_path = matches[3].str();

    int start_seconds = time_str_to_seconds(start_time_str);
    int end_seconds = time_str_to_seconds(end_time_str);
    int duration_seconds; // 先声明变量

    if (end_seconds < start_seconds) {// 这种情况处理跨天的时间记录 (例如 23:00 ~ 01:00)
        // 将结束持续的秒数加上一整天的秒数再计算差值
        duration_seconds = (end_seconds + 24 * 3600) - start_seconds;
    } else { // 没有跨天
        
        duration_seconds = end_seconds - start_seconds;
    }

    buffered_records_for_day.push_back({current_date, start_time_str, end_time_str, project_path, duration_seconds});
    _process_project_path(project_path);
}

void DataFileParser::_process_project_path(const std::string& project_path_orig) { 
    std::vector<std::string> segments = split_string(project_path_orig, '_');
    if (segments.empty()) return;
    for (const auto& pair : initial_top_level_parents) {
        parent_child_pairs.insert({pair.first, pair.second});
    }
    if (segments.size() > 1) {
        std::string parent_path = segments[0];
        for (size_t i = 1; i < segments.size(); ++i) {
            std::string child_path = parent_path + "_" + segments[i];
            parent_child_pairs.insert({child_path, parent_path});
            parent_path = child_path;
        }
    }
}

void DataFileParser::_store_previous_date_data() { 
    if (current_date.empty() || current_date_processed) return;
    days.push_back({current_date, current_status, current_sleep, current_remark, current_getup_time});
    records.insert(records.end(), buffered_records_for_day.begin(), buffered_records_for_day.end());
    buffered_records_for_day.clear();
    current_date_processed = true;
}