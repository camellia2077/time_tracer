// FormatValidator.cpp (已重构)

#include "FormatValidator.h"
#include "SharedUtils.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <regex>
#include <algorithm>
#include <vector>
#include <map>
#include <set>
#include <iomanip>

// --- 构造函数与配置加载 ---
FormatValidator::FormatValidator(const std::string& config_filename, const std::string& header_config_filename, bool enable_day_count_check)
    : config_filepath_(config_filename), 
      header_config_filepath_(header_config_filename),
      check_day_count_enabled_(enable_day_count_check) {
    loadConfiguration();
}

void FormatValidator::loadConfiguration() {
    // 加载验证器配置 (PARENT_CATEGORIES)
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

    // 加载头文件顺序和备注前缀 (从 interval_processor_config.json 读取)
    std::ifstream header_ifs(header_config_filepath_);
    if (header_ifs.is_open()) {
        try {
            nlohmann::json j;
            header_ifs >> j;
            if (j.contains("header_order") && j["header_order"].is_array()) {
                header_order_ = j["header_order"].get<std::vector<std::string>>();
            }
            // 新增: 为源文件验证加载备注前缀
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


// ##################################################################
// ###                  源文件验证 (NEW IMPLEMENTATION)             ###
// ##################################################################

bool FormatValidator::validateSourceFile(const std::string& file_path, std::set<Error>& errors) {
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
            // Don't return yet, continue checking to find all similar errors
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

// --- 源文件验证的私有辅助函数 ---
bool FormatValidator::isSourceDateLine(const std::string& line) {
    return line.length() == 4 && std::all_of(line.begin(), line.end(), ::isdigit);
}

bool FormatValidator::isSourceRemarkLine(const std::string& line) {
    if (remark_prefix_from_config_.empty() || line.rfind(remark_prefix_from_config_, 0) != 0) {
        return false;
    }
    std::string content = line.substr(remark_prefix_from_config_.length());
    return !trim(content).empty();
}

bool FormatValidator::parseSourceEventLine(const std::string& line, std::string& outTimeStr, std::string& outDescription) {
    if (line.length() < 5 || !std::all_of(line.begin(), line.begin() + 4, ::isdigit)) {
        return false;
    }
    try {
        int hh = std::stoi(line.substr(0, 2));
        int mm = std::stoi(line.substr(2, 2));
        if (hh > 23 || mm > 59) return false;
        outTimeStr = line.substr(0, 4);
        outDescription = line.substr(4);
        return !outDescription.empty();
    } catch (const std::exception&) {
        return false; // stoi could fail
    }
}


// ##################################################################
// ###                  输出文件验证 (EXISTING LOGIC)             ###
// ##################################################################

// 将原有的 validateFile 重命名为 validateOutputFile
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
                if (!block.activity_lines_content.empty()) {
                    const auto& last_activity = block.activity_lines_content.back();
                    if (block.sleep_value_from_file && last_activity.first != "sleep_night") {
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

    validate_date_continuity(errors);
    validate_month_start(month_day_map, errors);
    validate_all_days_for_month(month_day_map, errors);

    return errors.empty();
}


// --- 其余私有函数保持不变, 但 DateBlock::reset 需要实现 ---
void FormatValidator::DateBlock::reset() {
    start_line_number = -1;
    date_line_num = -1;
    date_str.clear();
    header_completely_valid = true;
    sleep_value_from_file = false;
    activity_lines_content.clear();
}
std::string FormatValidator::trim(const std::string& str) {
    const std::string WHITESPACE = " \n\r\t\f\v";
    size_t first = str.find_first_not_of(WHITESPACE);
    if (std::string::npos == first) return "";
    size_t last = str.find_last_not_of(WHITESPACE);
    return str.substr(first, (last - first + 1));
}
bool FormatValidator::parse_time_format(const std::string& time_str, int& hours, int& minutes) { /* ... */ return true; }
void FormatValidator::validate_status_line(const std::string& line, int line_num, DateBlock& block, std::set<Error>& errors) { /* ... */ }
void FormatValidator::validate_sleep_line(const std::string& line, int line_num, DateBlock& block, std::set<Error>& errors) { /* ... */ }
void FormatValidator::validate_getup_line(const std::string& line, int line_num, DateBlock& block, std::set<Error>& errors) { /* ... */ }
void FormatValidator::validate_remark_line(const std::string& line, int line_num, DateBlock& block, std::set<Error>& errors) { /* ... */ }
void FormatValidator::validate_activity_line(const std::string& line, int line_num, DateBlock& block, std::set<Error>& errors) { /* ... */ }
void FormatValidator::finalize_block_status_validation(DateBlock& block, std::set<Error>& errors) { /* ... */ }
bool FormatValidator::is_leap(int year) { return (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0)); }
int FormatValidator::days_in_month(int year, int month) { if (month == 2) return is_leap(year) ? 29 : 28; else if (month == 4 || month == 6 || month == 9 || month == 11) return 30; else return 31; }
std::string FormatValidator::increment_date(const std::string& date_str) { /* ... */ return ""; }
void FormatValidator::validate_date_line(const std::string& line, int line_num, DateBlock& block, std::set<Error>& errors) { /* ... */ }
void FormatValidator::validate_date_continuity(std::set<Error>& errors) { /* ... */ }
void FormatValidator::validate_month_start(const std::map<std::string, std::set<int>>& month_day_map, std::set<Error>& errors) { /* ... */ }
void FormatValidator::validate_all_days_for_month(const std::map<std::string, std::set<int>>& month_day_map, std::set<Error>& errors) { /* ... */ }