// --- START OF FILE parsing/data_parser.cpp ---

#include "data_parser.h"
#include <fstream>
#include <iostream>
#include <algorithm>
#include <stdexcept>

// --- DataFileParser 构造函数与析构函数 ---

DataFileParser::DataFileParser(const nlohmann::json& config_json)
    : current_date_processed(false),
      _time_record_regex(R"(([0-9]{2}:[0-9]{2})~([0-9]{2}:[0-9]{2})(.+))")
{
    _load_initial_parents(config_json);
}

DataFileParser::~DataFileParser() {
    // 析构函数通常为空，因为 commit_all() 应由外部调用
}

// --- 公共成员函数 (parse_file, commit_all 保持不变) ---
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


// --- 私有成员函数 ---

/**
 * @brief 【修改】此方法现在直接处理传入的json对象，不再关心文件IO。
 */
void DataFileParser::_load_initial_parents(const nlohmann::json& config_json) {
    try {
        // 检查 "initial_top_level_parents" 这个顶层键是否存在且为对象
        if (config_json.contains("initial_top_level_parents") && config_json["initial_top_level_parents"].is_object()) {
            const auto& parents_map = config_json["initial_top_level_parents"];
            for (auto& [key, value] : parents_map.items()) {
                if (value.is_string()) {
                    initial_top_level_parents[key] = value.get<std::string>();
                }
            }
        } else {
            // 如果JSON中没有这个键，只打印警告，不中断程序
            std::cerr << "Warning: Provided JSON config does not contain a valid 'initial_top_level_parents' object. Proceeding without initial parent mappings." << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "Warning: Error processing provided JSON config. Details: " << e.what() << std::endl;
    }
}

// --- 其余所有私有方法 (_process_lines, _process_single_line 等) 保持完全不变 ---
// (此处省略它们的完整代码，因为它们没有变化)
void DataFileParser::_process_lines(std::stringstream& buffer) {
    std::string line;
    int line_num = 0;
    while (std::getline(buffer, line)) {
        line_num++;
        _process_single_line(line, line_num);
    }
}

void DataFileParser::_process_single_line(const std::string& line, int line_num) {
    auto trimmed_line = line;
    trimmed_line.erase(0, trimmed_line.find_first_not_of(" \t\n\r\f\v"));
    trimmed_line.erase(trimmed_line.find_last_not_of(" \t\n\r\f\v") + 1);

    if (trimmed_line.empty()) return;

    if (trimmed_line.rfind("Date:", 0) == 0) {
        _store_previous_date_data();
        _handle_date_line(trimmed_line);
    } else if (trimmed_line.rfind("Status:", 0) == 0) {
        _handle_status_line(trimmed_line);
    } else if (trimmed_line.rfind("Sleep:", 0) == 0) {
        _handle_sleep_line(trimmed_line);
    } else if (trimmed_line.rfind("Remark:", 0) == 0) {
        _handle_remark_line(trimmed_line);
    } else if (trimmed_line.rfind("Getup:", 0) == 0) {
        _handle_getup_line(trimmed_line);
    } else if (trimmed_line.find('~') != std::string::npos) {
        _handle_time_record_line(trimmed_line, line_num);
    }
}

void DataFileParser::_handle_date_line(const std::string& line) {
    if (line.length() > 5) {
        current_date = line.substr(5);
        current_date.erase(0, current_date.find_first_not_of(" \t"));
        current_date.erase(current_date.find_last_not_of(" \t") + 1);
        current_status = "False";
        current_sleep = "False";
        current_remark = "";
        current_getup_time = "00:00";
        buffered_records_for_day.clear();
        current_date_processed = false;
    }
}

void DataFileParser::_handle_status_line(const std::string& line) {
    if (line.length() > 7) current_status = line.substr(7);
}

void DataFileParser::_handle_sleep_line(const std::string& line) {
    if (line.length() > 6) current_sleep = line.substr(6);
}

void DataFileParser::_handle_remark_line(const std::string& line) {
    if (line.length() > 7) current_remark = line.substr(7);
}

void DataFileParser::_handle_getup_line(const std::string& line) {
    if (line.length() > 6) current_getup_time = line.substr(6);
}

void DataFileParser::_handle_time_record_line(const std::string& line, int line_num) {
    std::smatch matches;
    if (std::regex_match(line, matches, _time_record_regex) && matches.size() == 4) {
        std::string start_time_str = matches[1].str();
        std::string end_time_str = matches[2].str();
        std::string project_path = matches[3].str();
        int start_seconds = time_str_to_seconds(start_time_str);
        int end_seconds = time_str_to_seconds(end_time_str);
        int duration_seconds = (end_seconds < start_seconds) ? ((end_seconds + 24 * 3600) - start_seconds) : (end_seconds - start_seconds);
        buffered_records_for_day.push_back({current_date, start_time_str, end_time_str, project_path, duration_seconds});
        _process_project_path(project_path);
    }
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