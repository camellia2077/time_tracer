#include "FormatValidator.h"
#include "common_utils.h" 
#include <iostream>
#include <fstream>
#include <sstream>
#include <regex>
#include <algorithm>
#include <vector>
#include <map>
#include <set>
#include <iomanip>

// --- 构造函数 ---
FormatValidator::FormatValidator(const std::string& config_filename, const std::string& header_config_filename, bool enable_day_count_check)
    : config_filepath_(config_filename), 
      header_config_filepath_(header_config_filename),
      check_day_count_enabled_(enable_day_count_check) {
    loadConfiguration();
}

// --- 配置加载 ---
void FormatValidator::loadConfiguration() {
    std::ifstream config_ifs(config_filepath_);
    if (config_ifs.is_open()) {
        try {
            nlohmann::json j;
            config_ifs >> j;
            if (j.contains("PARENT_CATEGORIES")) {
                config_.parent_categories = j["PARENT_CATEGORIES"].get<std::map<std::string, std::unordered_set<std::string>>>();
                config_.loaded = true;
            } else {
                 std::cerr << RED_COLOR << "Error: Validator config JSON does not contain 'PARENT_CATEGORIES' key." << RESET_COLOR << std::endl;
                 config_.loaded = false;
            }
        } catch (const std::exception& e) {
            std::cerr << RED_COLOR << "Error processing validator config JSON: " << e.what() << RESET_COLOR << std::endl;
            config_.loaded = false;
        }
    } else {
        std::cerr << RED_COLOR << "Error: Could not open validator config file: " << config_filepath_ << RESET_COLOR << std::endl;
    }

    std::ifstream header_ifs(header_config_filepath_);
    if (header_ifs.is_open()) {
        try {
            nlohmann::json j;
            header_ifs >> j;
            if (j.contains("header_order") && j["header_order"].is_array()) {
                header_order_ = j["header_order"].get<std::vector<std::string>>();
            }
            if (j.contains("remark_prefix") && j["remark_prefix"].is_string()) {
                remark_prefix_from_config_ = j["remark_prefix"].get<std::string>();
            }
        } catch (const std::exception& e) {
            std::cerr << RED_COLOR << "Error processing header format JSON: " << e.what() << RESET_COLOR << std::endl;
        }
    } else {
        std::cerr << RED_COLOR << "Error: Could not open header format file: " << header_config_filepath_ << RESET_COLOR << std::endl;
    }
}

// --- 源文件验证 ---
bool FormatValidator::validateSourceFile(const std::string& file_path, std::set<Error>& errors) {
    // ... (此部分实现与之前讨论的一致)
    std::ifstream inFile(file_path);
    if (!inFile.is_open()) {
        errors.insert({0, "Could not open file: " + file_path, ErrorType::FileAccess});
        return false;
    }
    std::string line;
    int lineNumber = 0;
    bool firstLineFound = false;
    bool eventFoundForCurrentDay = false;
    while (std::getline(inFile, line)) {
        lineNumber++;
        std::string trimmed_line = trim(line);
        if (trimmed_line.empty()) continue;
        if (isSourceDateLine(trimmed_line)) {
            eventFoundForCurrentDay = false;
            if (!firstLineFound) firstLineFound = true;
            continue;
        }
        if (!firstLineFound) {
            errors.insert({lineNumber, "The first non-empty line must be a 4-digit date. Found: '" + trimmed_line + "'", ErrorType::Source_NoDateAtStart});
            continue;
        }
        if (isSourceRemarkLine(trimmed_line)) {
            if (eventFoundForCurrentDay) {
                errors.insert({lineNumber, "Remark lines cannot appear after an event line for the same day. Found: '" + trimmed_line + "'", ErrorType::Source_RemarkAfterEvent});
            }
            continue;
        }
        std::string timeStr, description;
        if (parseSourceEventLine(trimmed_line, timeStr, description)) {
            eventFoundForCurrentDay = true;
            continue;
        }
        errors.insert({lineNumber, "Invalid format. Must be a date (MMDD), remark (e.g., 'r text'), or event (e.g., '0830event'). Found: '" + trimmed_line + "'", ErrorType::Source_InvalidLineFormat});
    }
    return errors.empty();
}

// --- 输出文件验证 ---
bool FormatValidator::validateOutputFile(const std::string& file_path, std::set<Error>& errors) {
    std::ifstream file(file_path);
    if (!file.is_open()) {
        errors.insert({0, "Could not open file: " + file_path, ErrorType::FileAccess});
        return false;
    }
    std::string line;
    int line_number = 0;
    DateBlock current_block;
    size_t expected_header_idx = 0;
    bool in_activity_section = false;
    collected_dates_.clear();
    std::map<std::string, std::set<int>> month_day_map;

    auto finalize_previous_block = [&](DateBlock& block) {
        if (block.date_line_num != -1) {
            if (!block.date_str.empty()) {
                std::string yyyymm = block.date_str.substr(0, 6);
                int day = std::stoi(block.date_str.substr(6, 2));
                month_day_map[yyyymm].insert(day);
            }
            if (block.header_completely_valid) {
                finalize_block_status_validation(block, errors);

                // 【核心修正】在这里检查最后一个活动
                if (!block.activity_lines_content.empty()) {
                    const auto& last_activity = block.activity_lines_content.back();
                    // last_activity.first 现在是活动名称
                    if (block.sleep_value_from_file && last_activity.first != "sleep_night") {
                        // last_activity.second 是行号
                        errors.insert({last_activity.second, "The last activity of the day must be 'sleep_night'.", ErrorType::MissingSleepNight});
                    }
                }
            }
            block.reset();
        }
    };

    while (std::getline(file, line)) {
        line_number++;
        std::string trimmed_line = trim(line);
        if (trimmed_line.empty()) continue;
        if (trimmed_line.rfind("Date:", 0) == 0) {
            finalize_previous_block(current_block);
            expected_header_idx = 0;
            in_activity_section = false;
            current_block.start_line_number = line_number;
        }
        if (current_block.start_line_number == -1) {
            errors.insert({line_number, "File must begin with a valid Date: line.", ErrorType::Structural});
            continue;
        }
        if (!current_block.header_completely_valid) continue;
        if (!in_activity_section) {
            if (expected_header_idx >= header_order_.size()) {
                if(header_order_.back() == "Remark:"){
                    in_activity_section = true;
                    validate_activity_line(trimmed_line, line_number, current_block, errors);
                } else {
                    errors.insert({line_number, "Unexpected line after header section.", ErrorType::Structural});
                    current_block.header_completely_valid = false;
                }
                continue;
            }
            const std::string& expected_header = header_order_[expected_header_idx];
            if (trimmed_line.rfind(expected_header, 0) == 0) {
                if (expected_header == "Date:") validate_date_line(trimmed_line, line_number, current_block, errors);
                else if (expected_header == "Status:") validate_status_line(trimmed_line, line_number, current_block, errors);
                else if (expected_header == "Sleep:") validate_sleep_line(trimmed_line, line_number, current_block, errors);
                else if (expected_header == "Getup:") validate_getup_line(trimmed_line, line_number, current_block, errors);
                else if (expected_header == "Remark:") validate_remark_line(trimmed_line, line_number, current_block, errors);
                expected_header_idx++;
            } else {
                errors.insert({line_number, "Line out of order. Expected '" + expected_header + "'.", ErrorType::Structural});
                current_block.header_completely_valid = false;
            }
        } else {
            validate_activity_line(trimmed_line, line_number, current_block, errors);
        }
    }
    finalize_previous_block(current_block);
    if(check_day_count_enabled_){
        validate_date_continuity(errors);
        validate_month_start(month_day_map, errors);
        validate_all_days_for_month(month_day_map, errors);
    }
    return errors.empty();
}
// --- 错误报告 ---
void FormatValidator::printGroupedErrors(const std::string& filename, const std::set<Error>& errors, const std::string& error_log_path) {
    // ... (此部分实现与之前讨论的一致)
    std::cerr << "请根据以下错误信息，手动修正该文件。" << std::endl;
    std::map<ErrorType, std::vector<Error>> grouped_errors;
    for (const auto& err : errors) {
        ErrorType group_key = err.type;
        if (err.type == ErrorType::DateContinuity) { group_key = ErrorType::TimeDiscontinuity; }
        grouped_errors[group_key].push_back(err);
    }
    std::ofstream err_stream(error_log_path, std::ios::app);
    err_stream << "\n文件 " << filename << " 的检验错误\n--------------------------------------------------\n\n";
    for (const auto& pair : grouped_errors) {
        std::string header = getErrorTypeHeader(pair.first);
        std::cerr << "\n" << header << std::endl;
        err_stream << header << "\n";
        for (const auto& err : pair.second) {
            std::string error_message = (err.line_number == 0) ? err.message : "Line " + std::to_string(err.line_number) + ": " + err.message;
            std::cerr << "  " << error_message << std::endl;
            err_stream << "  " << error_message << "\n";
        }
    }
    err_stream.close();
    std::cout << "\n详细的错误日志已保存至: " << YELLOW_COLOR << error_log_path << RESET_COLOR << std::endl;
}

// --- 私有辅助函数实现 ---
// (包含 is_valid_activity 和 validate_activity_line 的完整实现)
void FormatValidator::DateBlock::reset() {
    start_line_number = -1; 
    date_line_num = -1; 
    date_str.clear();
    header_completely_valid = true; 
    sleep_value_from_file = false;
    activity_lines_content.clear();
    last_activity_end_time.clear();
}
std::string FormatValidator::trim(const std::string& str) {
    const std::string WHITESPACE = " \n\r\t\f\v";
    size_t first = str.find_first_not_of(WHITESPACE);
    if (std::string::npos == first) return "";
    size_t last = str.find_last_not_of(WHITESPACE);
    return str.substr(first, (last - first + 1));
}
bool FormatValidator::isSourceDateLine(const std::string& line) { return line.length() == 4 && std::all_of(line.begin(), line.end(), ::isdigit); }
bool FormatValidator::isSourceRemarkLine(const std::string& line) {
    if (remark_prefix_from_config_.empty() || line.rfind(remark_prefix_from_config_, 0) != 0) return false;
    return !trim(line.substr(remark_prefix_from_config_.length())).empty();
}
bool FormatValidator::parseSourceEventLine(const std::string& line, std::string& outTimeStr, std::string& outDescription) {
    if (line.length() < 5 || !std::all_of(line.begin(), line.begin() + 4, ::isdigit)) return false;
    try {
        int hh = std::stoi(line.substr(0, 2)); int mm = std::stoi(line.substr(2, 2));
        if (hh > 23 || mm > 59) return false;
        outTimeStr = line.substr(0, 4); outDescription = line.substr(4);
        return !outDescription.empty();
    } catch (const std::exception&) { return false; }
}
bool FormatValidator::parse_time_format(const std::string& time_str, int& hours, int& minutes) {
    if (time_str.length() != 5 || time_str[2] != ':') return false;
    try {
        hours = std::stoi(time_str.substr(0, 2)); minutes = std::stoi(time_str.substr(3, 2));
        return (hours >= 0 && hours <= 23 && minutes >= 0 && minutes <= 59);
    } catch (...) { return false; }
}
void FormatValidator::validate_date_line(const std::string& line, int line_num, DateBlock& block, std::set<Error>& errors) {
    std::string date = line.substr(5);
    if (date.length() == 8 && std::all_of(date.begin(), date.end(), ::isdigit)) {
        block.date_str = date; block.date_line_num = line_num;
        collected_dates_.push_back({date, line_num});
    } else {
        errors.insert({line_num, "Invalid Date format. Expected 'Date:YYYYMMDD'", ErrorType::LineFormat});
        block.header_completely_valid = false;
    }
}
void FormatValidator::validate_status_line(const std::string& line, int line_num, DateBlock& block, std::set<Error>& errors) { /* Placeholder */ }
void FormatValidator::validate_sleep_line(const std::string& line, int line_num, DateBlock& block, std::set<Error>& errors) {
    if (line == "Sleep:True") block.sleep_value_from_file = true;
    else if (line != "Sleep:False") {
        errors.insert({line_num, "Invalid Sleep format. Expected 'Sleep:True' or 'Sleep:False'", ErrorType::LineFormat});
        block.header_completely_valid = false;
    }
}
void FormatValidator::validate_getup_line(const std::string& line, int line_num, DateBlock& block, std::set<Error>& errors) { /* Placeholder */ }
void FormatValidator::validate_remark_line(const std::string& line, int line_num, DateBlock& block, std::set<Error>& errors) { /* Placeholder */ }
void FormatValidator::finalize_block_status_validation(DateBlock& block, std::set<Error>& errors) { /* Placeholder */ }
bool FormatValidator::is_leap(int year) { return (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0)); }
int FormatValidator::days_in_month(int year, int month) { if (month == 2) return is_leap(year) ? 29 : 28; else if (month == 4 || month == 6 || month == 9 || month == 11) return 30; else return 31; }
std::string FormatValidator::increment_date(const std::string& date_str) { /* Placeholder */ return ""; }
void FormatValidator::validate_date_continuity(std::set<Error>& errors) { /* Placeholder */ }
void FormatValidator::validate_month_start(const std::map<std::string, std::set<int>>& month_day_map, std::set<Error>& errors) { /* Placeholder */ }
void FormatValidator::validate_all_days_for_month(const std::map<std::string, std::set<int>>& month_day_map, std::set<Error>& errors) { /* Placeholder */ }
std::string FormatValidator::getErrorTypeHeader(ErrorType type) {
    switch (type) {
        case ErrorType::Source_RemarkAfterEvent: case ErrorType::Source_NoDateAtStart: case ErrorType::Source_InvalidLineFormat: return "Source file format errors (源文件格式错误):";
        case ErrorType::IncorrectDayCountForMonth: return "Date errors(日期错误):";
        case ErrorType::DateContinuity: case ErrorType::TimeDiscontinuity: return "Time discontinuity errors(时间不连续):";
        case ErrorType::MissingSleepNight: return "Lack of sleep_night errors(最后的活动项目缺少sleep_night):";
        case ErrorType::FileAccess: return "File access errors:"; case ErrorType::Structural: return "Structural errors:";
        case ErrorType::LineFormat: return "Line format errors:"; case ErrorType::Logical: return "Logical consistency errors:";
        default: return "Other errors:";
    }
}
bool FormatValidator::is_valid_activity(const std::string& activity_name) const {
    if (!config_.loaded) return true;
    for (const auto& parent_pair : config_.parent_categories) {
        if (parent_pair.second.count(activity_name)) {
            return true;
        }
    }
    return false;
}
void FormatValidator::validate_activity_line(const std::string& line, int line_num, DateBlock& block, std::set<Error>& errors) {
    size_t tilde_pos = line.find('~');
    if (tilde_pos == std::string::npos || tilde_pos == 0) {
        errors.insert({line_num, "Invalid activity format. Expected 'HH:MM~HH:MMactivity'.", ErrorType::LineFormat});
        return;
    }
    std::string start_time_str = line.substr(0, tilde_pos);
    std::string rest_of_line = line.substr(tilde_pos + 1);
    if (rest_of_line.length() < 5) {
        errors.insert({line_num, "Invalid activity format after '~'. Expected 'HH:MMactivity'.", ErrorType::LineFormat});
        return;
    }
    std::string end_time_str = rest_of_line.substr(0, 5);
    std::string activity_name = rest_of_line.substr(5);
    int start_h, start_m, end_h, end_m;
    if (!parse_time_format(start_time_str, start_h, start_m) || !parse_time_format(end_time_str, end_h, end_m)) {
        errors.insert({line_num, "Invalid time format in activity line.", ErrorType::LineFormat});
        return;
    }
    
    // 【核心修正】时间连续性检查
    if (!block.last_activity_end_time.empty()) {
        if (start_time_str != block.last_activity_end_time) {
            errors.insert({line_num, "Time discontinuity. Expected start time " + block.last_activity_end_time + ", but got " + start_time_str + ".", ErrorType::TimeDiscontinuity});
        }
    }
    
    // 活动名称有效性检查
    if (!is_valid_activity(activity_name)) {
        errors.insert({line_num, "Unknown activity '" + activity_name + "'. It is not defined in the format validator configuration.", ErrorType::Logical});
    }

    // 【核心修正】存储正确的数据
    // 存储 {活动名称, 行号}
    block.activity_lines_content.push_back({activity_name, line_num});
    // 更新上一个活动的结束时间
    block.last_activity_end_time = end_time_str;
}