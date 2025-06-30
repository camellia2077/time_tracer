#include "data_parser.h"
#include <fstream>
#include <iostream>
#include <algorithm>
#include <stdexcept>
// 调用 common_utils.h 中的 time_str_to_seconds 函数

// --- DataFileParser 构造函数与析构函数 ---

/**
 * @brief DataFileParser 的构造函数实现。
 * @details 初始化成员变量，包括设置用于解析的正则表达式，并调用私有方法从JSON文件加载初始的父级项目映射。
 * @param config_path 指向JSON配置文件的路径。
 */
DataFileParser::DataFileParser(const std::string& config_path)
    : current_date_processed(false), // 初始化 current_date_processed 为 false
    // [修改] 将 \d 替换为 [0-9] 以提高正则表达式的兼容性
    // [兼容性修复] 将 \d 替换为 [0-9]。
    // 原因：一些C++标准库实现（尤其是一些MinGW-GCC版本）对 \d 的支持较差，
    // 这可能会在初始化期间导致 'std::regex_error' 异常。使用 [0-9] 是一种更健壮、更可移植的方式。
    // 警告：不要更改此正则表达式 — 它匹配用于 "HH:MM~HH:MM事件" 格式时间记录行的预期输入格式
    _time_record_regex(R"(([0-9]{2}:[0-9]{2})~([0-9]{2}:[0-9]{2})(.+))")
    {
    _load_initial_parents(config_path);
}

/**
 * @brief DataFileParser 的析构函数实现。
 * @details 析构函数本身不执行特殊逻辑。重要的是，类的使用者应在所有文件解析完成后手动调用 `commit_all()`，以确保最后一个文件中的数据得到处理。
 */
DataFileParser::~DataFileParser() {
    // commit_all() 方法应由所有者显式调用
    // 以确保处理完最后一个文件中的任何残留数据。
    // 析构函数，没有特定的清理逻辑，因为 commit_all() 应由外部调用方显式调用
}

// --- 公共成员函数 ---

/**
 * @brief 提交所有缓冲的数据。
 * @details 此函数确保最后一个日期的数据被正确存储。它通过调用 `_store_previous_date_data` 来完成此操作。
 */
void DataFileParser::commit_all() {
    // 存储处理的最后一个文件中最后日期的任何残留数据。
    _store_previous_date_data();
}

/**
 * @brief 解析文件的核心公共方法。
 * @details 此函数负责打开和读取文件，然后启动行处理流程。它将整个文件读入一个字符串流以提高效率，并设置了异常处理机制以捕获解析错误。
 * @param filename 要解析的文件的路径。
 * @return 如果解析成功，返回 true，否则返回 false。
 */
bool DataFileParser::parse_file(const std::string& filename) {
    std::ifstream file(filename); // 创建一个输入文件流对象并尝试打开指定的文件
    if (!file.is_open()) { // 如果文件打开失败
        std::cerr << "Error: Cannot open file " << filename << std::endl; // 向标准错误流 std::cerr 输出错误消息
        return false; // 返回 false 表示失败
    }

    std::stringstream buffer; // 创建一个字符串流
    buffer << file.rdbuf(); // 为了高效处理，一次性将整个文件内容读入字符串流缓冲区
    file.close(); // 关闭文件
    current_file_name = filename; // 保存当前文件名，主要用于在发生错误时提供上下文。
    
    bool success = true; // 初始化成功状态为 true
    try { // 使用 try-catch 块捕获解析过程中可能发生的任何标准异常，确保程序的健壮性。
        _process_lines(buffer); // 调用一个私有方法来处理所有行
    } catch (const std::exception& e) { // 捕获标准异常
        std::cerr << current_file_name << ": An error occurred during parsing: " << e.what() << std::endl; // 输出错误消息
        success = false; // 设置成功状态为 false
    }

    return success; // 返回最终的成功状态
}


// --- 私有成员函数 ---

/**
 * @brief 从JSON配置文件加载初始的父级映射。
 * @details 此函数打开并解析一个JSON文件，该文件定义了某些项目（键）及其预设的父项目（值）。
 * 例如 `{"study": "mystudy"}`。如果文件不存在或解析失败，它会打印警告但不会中止程序。
 * @param config_path JSON配置文件的路径。
 */
void DataFileParser::_load_initial_parents(const std::string& config_path) {
    std::ifstream config_file(config_path);
    if (!config_file.is_open()) {
        std::cerr << "Warning: Cannot open configuration file " << config_path << ". Proceeding without initial parent mappings." << std::endl;
        return;
    }

    try {
        nlohmann::json json_data;
        config_file >> json_data;
        for (auto& [key, value] : json_data.items()) {
            if (value.is_string()) {
                initial_top_level_parents[key] = value.get<std::string>();
            }
        }
    } catch (const nlohmann::json::parse_error& e) {
        std::cerr << "Warning: Failed to parse JSON from " << config_path << ". Proceeding without initial parent mappings. Details: " << e.what() << std::endl;
    }
}

/**
 * @brief 逐行处理字符串流中的内容。
 * @details 此函数循环遍历传入的字符串流，每次读取一行，并将其传递给 `_process_single_line` 进行具体处理。
 * @param buffer 包含文件所有内容的字符串流。
 */
void DataFileParser::_process_lines(std::stringstream& buffer) {
    std::string line; // 用于存储每行的内容
    int line_num = 0; // 初始化行号计数器
    
    while (std::getline(buffer, line)) { // 逐行从字符串流缓冲区读取内容
        line_num++; // 增加行号
        _process_single_line(line, line_num); // 调用单行处理函数
    }
}

/**
 * @brief 对单行内容进行预处理和分派。
 * @details 此函数首先去除行首和行尾的空白字符。然后，它检查该行是否匹配某种已知的模式（如以 "Date:" 开头），
 * 并调用相应的私有处理函数（如 `_handle_date_line`）来处理它。
 * @param line 要处理的原始行字符串。
 * @param line_num 当前行号，保留用于调试。
 */
void DataFileParser::_process_single_line(const std::string& line, int line_num) {
    auto trimmed_line = line; // 复制行内容以进行修改
    trimmed_line.erase(0, trimmed_line.find_first_not_of(" \t\n\r\f\v")); // 删除行首的所有空白字符
    trimmed_line.erase(trimmed_line.find_last_not_of(" \t\n\r\f\v") + 1); // 删除行尾的所有空白字符

    if (trimmed_line.empty()) return; // 如果修剪后的行为空，则忽略它

    // 根据行前缀分派到不同的处理函数
    if (trimmed_line.rfind("Date:", 0) == 0) { // 如果行以 "Date:" 开头
        _store_previous_date_data();  // 当遇到新的 "Date:" 行时，意味着前一天的数据已完整，因此需要先存储前一天所有缓存的数据。
        _handle_date_line(trimmed_line);
    } else if (trimmed_line.rfind("Status:", 0) == 0) { // 如果行以 "Status:" 开头
        _handle_status_line(trimmed_line);
    } else if (trimmed_line.rfind("Sleep:", 0) == 0) { // 新增：处理 Sleep 行
        _handle_sleep_line(trimmed_line);
    } else if (trimmed_line.rfind("Remark:", 0) == 0) { // 如果行以 "Remark:" 开头
        _handle_remark_line(trimmed_line);
    } else if (trimmed_line.rfind("Getup:", 0) == 0) { // 如果行以 "Getup:" 开头
        _handle_getup_line(trimmed_line);
    } else if (trimmed_line.find('~') != std::string::npos) { // 如果行包含“~”字符，则视其为时间记录行
        _handle_time_record_line(trimmed_line, line_num);
    }
}

/**
 * @brief 处理包含日期信息的行。
 * @details 当此函数被调用时，它首先提取日期字符串，然后重置所有与天相关的状态变量（如状态、备注、睡眠等），为解析新一天的数据做准备。
 * @param line 以 "Date:" 开头的行。
 */
void DataFileParser::_handle_date_line(const std::string& line) {
    if (line.length() > 5) { // 确保行足够长以提取日期
        current_date = line.substr(5); // 提取 "Date:" 之后的子字符串作为日期
        current_date.erase(0, current_date.find_first_not_of(" \t")); // 删除日期字符串开头的多余空格
        current_date.erase(current_date.find_last_not_of(" \t") + 1); // 删除日期字符串末尾的多余空格

        // 为新的一天重置
        current_status = "False"; // 将状态重置为 "False"
        current_sleep = "False";  // 新增：将睡眠状态重置为 "False"
        current_remark = ""; // 将备注重置为空字符串
        current_getup_time = "00:00"; // 将起床时间重置为 "00:00"
        buffered_records_for_day.clear(); // 清除当天的​​时间记录缓冲区
        current_date_processed = false; // 将当前日期标记为未处理
    }
}

/**
 * @brief 处理包含状态信息的行。
 * @param line 以 "Status:" 开头的行。
 */
void DataFileParser::_handle_status_line(const std::string& line) {
    if (line.length() > 7) {
        current_status = line.substr(7); // 提取 "Status:" 之后的子字符串作为状态
    }
}

/**
 * @brief 处理包含睡眠状态信息的行。
 * @param line 以 "Sleep:" 开头的行。
 */
void DataFileParser::_handle_sleep_line(const std::string& line) {
    if (line.length() > 6) {
        current_sleep = line.substr(6); // 提取 "Sleep:" 之后的子字符串作为状态
    }
}

/**
 * @brief 处理包含备注信息的行。
 * @param line 以 "Remark:" 开头的行。
 */
void DataFileParser::_handle_remark_line(const std::string& line) {
    if (line.length() > 7) {
        current_remark = line.substr(7); // 提取 "Remark:" 之后的子字符串作为备注
    }
}

/**
 * @brief 处理包含起床时间信息的行。
 * @param line 以 "Getup:" 开头的行。
 */
void DataFileParser::_handle_getup_line(const std::string& line) {
    if (line.length() > 6) {
        current_getup_time = line.substr(6); // 提取 "Getup:" 之后的子字符串作为起床时间
    }
}

/**
 * @brief 使用正则表达式处理时间记录行。
 * @details 此函数匹配 "HH:MM~HH:MM事件" 格式，提取时间和事件，计算持续秒数，
 * 然后将这些信息封装成一个 `TimeRecordInternal` 对象并存入当天的缓冲区。
 * @param line 包含 "~" 的时间记录行。
 * @param line_num 当前行号，保留用于调试。
 */
void DataFileParser::_handle_time_record_line(const std::string& line, int line_num) {
    std::smatch matches; // 用于存储正则表达式匹配的结果
    if (std::regex_match(line, matches, _time_record_regex) && matches.size() == 4) 
    { 
        std::string start_time_str = matches[1].str(); // 提取开始时间字符串
        std::string end_time_str = matches[2].str();   // 提取结束时间字符串
        std::string project_path = matches[3].str();   // 提取项目路径字符串

        int start_seconds = time_str_to_seconds(start_time_str); // 将开始时间转换为秒
        int end_seconds = time_str_to_seconds(end_time_str);     // 将结束时间转换为秒
        
        // 计算以秒为单位的持续时间，处理跨午夜的情况（结束时间小于开始时间）
        int duration_seconds = (end_seconds < start_seconds) ? ((end_seconds + 24 * 3600) - start_seconds) : (end_seconds - start_seconds);

        // 将解析的时间记录存储在每日缓冲区中
        buffered_records_for_day.push_back({current_date, start_time_str, end_time_str, project_path, duration_seconds});
        _process_project_path(project_path); // 处理项目路径以建立父子关系
    }
}

/**
 * @brief 解析项目路径以建立层级化的父子关系。
 * @details 对于用下划线分隔的路径（如 "a_b_c"），此函数会生成一系列的父子关系对
 * （如 "a_b" 是 "a_b_c" 的父项，"a" 是 "a_b" 的父项），并存储它们。
 * 它还会利用 `initial_top_level_parents` 映射来处理顶层项目的关系。
 * @param project_path_orig 从时间记录行中解析出的原始项目路径。
 */
void DataFileParser::_process_project_path(const std::string& project_path_orig) {
    std::vector<std::string> segments = split_string(project_path_orig, '_');

    if (segments.empty()) {
        return; // 如果没有分段，则直接返回
    }

    // 将预定义的顶层父子关系添加到整个父子关系集合中
    // 这个循环每次调用此函数时都会运行，但由于 parent_child_pairs 是一个集合，重复插入没有副作用。
    for (const auto& pair : initial_top_level_parents) {
        parent_child_pairs.insert({pair.first, pair.second});
    }

    // 仅当路径包含多个段时才创建层次关系。
    if (segments.size() > 1) {
        std::string parent_path = segments[0];
        for (size_t i = 1; i < segments.size(); ++i) {
            std::string child_path = parent_path + "_" + segments[i];
            parent_child_pairs.insert({child_path, parent_path});
            parent_path = child_path; // 对于下一段，当前子路径成为父路径
        }
    }
}

/**
 * @brief 将前一天缓冲的数据永久存储到主数据向量中。
 * @details 此函数被调用时，它会将 `current_date` 的所有信息（包括 `DayData` 和 `buffered_records_for_day` 中的所有记录）
 * 分别移动到 `days` 和 `records` 向量中。然后它会清空缓冲区并标记该日期为已处理。
 */
void DataFileParser::_store_previous_date_data() {
    // 如果当前日期为空（文件开头）或当前日期的数据已被处理，则直接返回
    if (current_date.empty() || current_date_processed) return;

    // 将收集的日期信息添加到主“days”向量中
    days.push_back({current_date, current_status, current_sleep, current_remark, current_getup_time});

    // 将当天所有缓冲的时间记录添加到主“records”向量中
    records.insert(records.end(), buffered_records_for_day.begin(), buffered_records_for_day.end());

    buffered_records_for_day.clear(); // 清除每日记录缓冲区，为第二天做准备
    current_date_processed = true; // 将当前日期的数据标记为已处理
}