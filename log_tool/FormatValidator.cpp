// FormatValidator.cpp

#include "FormatValidator.h"
#include "SharedUtils.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <regex>
#include <algorithm>
#include <vector>

// 修改：DateBlock 结构体增加 sleep 相关字段
struct FormatValidator::DateBlock {
    int start_line_number = -1;
    std::string date_str;
    int date_line_num = -1;
    bool status_value_from_file = false;
    bool status_format_valid = false;
    int status_line_num = -1;
    bool sleep_format_valid = false; // 新增
    int sleep_line_num = -1; // 新增
    std::vector<std::pair<std::string, int>> activity_lines_content;
    bool header_completely_valid = true;
    std::string last_activity_end_time;

    void reset() {
        start_line_number = -1;
        date_str.clear();
        date_line_num = -1;
        status_value_from_file = false;
        status_format_valid = false;
        status_line_num = -1;
        sleep_format_valid = false; // 新增
        sleep_line_num = -1; // 新增
        activity_lines_content.clear();
        header_completely_valid = true;
        last_activity_end_time.clear();
    }
};

// 修改：构造函数接收两个配置文件
FormatValidator::FormatValidator(const std::string& config_filename, const std::string& header_config_filename)
    : config_filepath_(config_filename), header_config_filepath_(header_config_filename) {
    loadConfiguration();
    if (config_.loaded && !header_order_.empty()) {
        std::cout << GREEN_COLOR << "Format validator configurations loaded successfully." << RESET_COLOR << std::endl;
    } else {
        std::cout << YELLOW_COLOR << "Warning: Format validator running with limited/default validation." << RESET_COLOR << std::endl;
    }
}

// 重写：validateFile 使用新的动态状态机
bool FormatValidator::validateFile(const std::string& file_path, std::set<Error>& errors) {
    std::ifstream file(file_path);
    if (!file.is_open()) {
        errors.insert({0, "Could not open file: " + file_path, ErrorType::FileAccess});
        return false;
    }

    std::string line;
    int line_number = 0;
    DateBlock current_block;
    
    // 动态状态机状态
    size_t expected_header_idx = 0; // 0对应header_order_的第一个元素
    bool in_activity_section = false;

    auto finalize_previous_block = [&](DateBlock& block) {
        if (block.date_line_num != -1) {
            if (block.header_completely_valid) {
                finalize_block_status_validation(block, errors);
                if (!block.activity_lines_content.empty()) {
                    const auto& last_activity = block.activity_lines_content.back();
                    if (last_activity.first != "sleep_night") {
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
                 // 理论上不应该到这里，除非 Remark 后面还有非活动行
                errors.insert({line_number, "Unexpected line after header section.", ErrorType::Structural});
                current_block.header_completely_valid = false;
                continue;
            }

            const std::string& expected_header = header_order_[expected_header_idx];
            if (trimmed_line.rfind(expected_header, 0) == 0) {
                // 根据期望的头部调用相应的验证函数
                if (expected_header == "Date:") validate_date_line(trimmed_line, line_number, current_block, errors);
                else if (expected_header == "Status:") validate_status_line(trimmed_line, line_number, current_block, errors);
                else if (expected_header == "Sleep:") validate_sleep_line(trimmed_line, line_number, current_block, errors);
                else if (expected_header == "Getup:") validate_getup_line(trimmed_line, line_number, current_block, errors);
                else if (expected_header == "Remark:") {
                    validate_remark_line(trimmed_line, line_number, current_block, errors);
                    in_activity_section = true; // Remark之后就是活动列表
                }
                expected_header_idx++;
            } else {
                errors.insert({line_number, "Line out of order. Expected '" + expected_header + "'.", ErrorType::Structural});
                current_block.header_completely_valid = false; // 停止处理这个块的头部
            }
        } else {
            // 在活动区域
            validate_activity_line(trimmed_line, line_number, current_block, errors);
        }
    }

    finalize_previous_block(current_block);

    return errors.empty();
}


// 修改：加载两个配置文件
void FormatValidator::loadConfiguration() {
    // 1. 加载主配置
    std::ifstream ifs(config_filepath_);
    if (ifs.is_open()) {
        try {
            nlohmann::json j;
            ifs >> j;
            if (j.contains("PARENT_CATEGORIES") && j["PARENT_CATEGORIES"].is_object()) {
                config_.parent_categories = j["PARENT_CATEGORIES"].get<std::map<std::string, std::unordered_set<std::string>>>();
                config_.loaded = true;
            }
        } catch (const std::exception& e) {
            std::cerr << YELLOW_COLOR << "Warning: Error processing config file '" << config_filepath_ << "': " << e.what() << RESET_COLOR << std::endl;
        }
    }

    // 2. 加载头部顺序配置
    std::ifstream header_ifs(header_config_filepath_);
    if (!header_ifs.is_open()) {
        std::cerr << RED_COLOR << "Fatal Error: Header format config file not found: " << header_config_filepath_ << RESET_COLOR << std::endl;
        return;
    }
    try {
        nlohmann::json j;
        header_ifs >> j;
        if (j.contains("header_order") && j["header_order"].is_array()) {
            header_order_ = j["header_order"].get<std::vector<std::string>>();
        } else {
            throw std::runtime_error("Header config missing 'header_order' array.");
        }
    } catch (const std::exception& e) {
        std::cerr << RED_COLOR << "Fatal Error processing header config: " << e.what() << RESET_COLOR << std::endl;
        header_order_.clear(); // 清空以示失败
    }
}

// ... trim 和 parse_time_format 保持不变 ...
std::string FormatValidator::trim(const std::string& str) {
    const std::string whitespace = " \t\n\r\f\v";
    size_t start = str.find_first_not_of(whitespace);
    if (start == std::string::npos) return "";
    size_t end = str.find_last_not_of(whitespace);
    return str.substr(start, end - start + 1);
}

bool FormatValidator::parse_time_format(const std::string& time_str, int& hours, int& minutes) {
    if (time_str.length() != 5 || time_str[2] != ':') return false;
    try {
        hours = std::stoi(time_str.substr(0, 2));
        minutes = std::stoi(time_str.substr(3, 2));
        return (hours >= 0 && hours <= 23 && minutes >= 0 && minutes <= 59);
    } catch (...) {
        return false;
    }
}


// ... validate_date_line, validate_status_line, validate_getup_line, validate_remark_line, validate_activity_line, finalize_block_status_validation 等函数保持不变，但新增 validate_sleep_line
void FormatValidator::validate_date_line(const std::string& line, int line_num, DateBlock& block, std::set<Error>& errors) {
    block.date_line_num = line_num;
    static const std::regex date_regex("^Date:(\\d{8})$");
    std::smatch match;
    if (std::regex_match(line, match, date_regex)) {
        block.date_str = match[1].str();
    } else {
        errors.insert({line_num, "Date line format error. Expected 'Date:YYYYMMDD'.", ErrorType::LineFormat});
        block.header_completely_valid = false;
    }
}

void FormatValidator::validate_status_line(const std::string& line, int line_num, DateBlock& block, std::set<Error>& errors) {
    block.status_line_num = line_num;
    if (line == "Status:True") {
        block.status_value_from_file = true;
        block.status_format_valid = true;
    } else if (line == "Status:False") {
        block.status_value_from_file = false;
        block.status_format_valid = true;
    } else {
        errors.insert({line_num, "Status line format error. Expected 'Status:True' or 'Status:False'.", ErrorType::LineFormat});
        block.status_format_valid = false;
        block.header_completely_valid = false;
    }
}

// 新增：validate_sleep_line 实现
void FormatValidator::validate_sleep_line(const std::string& line, int line_num, DateBlock& block, std::set<Error>& errors) {
    block.sleep_line_num = line_num;
    if (line == "Sleep:True" || line == "Sleep:False") {
        block.sleep_format_valid = true;
    } else {
        errors.insert({line_num, "Sleep line format error. Expected 'Sleep:True' or 'Sleep:False'.", ErrorType::LineFormat});
        block.sleep_format_valid = false;
        block.header_completely_valid = false;
    }
}

void FormatValidator::validate_getup_line(const std::string& line, int line_num, DateBlock& block, std::set<Error>& errors) {
    static const std::regex getup_regex("^Getup:(\\d{2}:\\d{2})$");
    std::smatch match;
    if (std::regex_match(line, match, getup_regex)) {
        int hh, mm;
        if (!parse_time_format(match[1].str(), hh, mm)) {
            errors.insert({line_num, "Getup time value invalid.", ErrorType::LineFormat});
            block.header_completely_valid = false;
        }
    } else {
        errors.insert({line_num, "Getup line format error. Expected 'Getup:HH:MM'.", ErrorType::LineFormat});
        block.header_completely_valid = false;
    }
}

void FormatValidator::validate_remark_line(const std::string& line, int line_num, DateBlock& block, std::set<Error>& errors) {
    if (line.rfind("Remark:", 0) != 0) {
        errors.insert({line_num, "Remark line format error. Expected 'Remark:' prefix.", ErrorType::LineFormat});
        block.header_completely_valid = false;
    }
}

void FormatValidator::validate_activity_line(const std::string& line, int line_num, DateBlock& block, std::set<Error>& errors) {
    static const std::regex activity_regex("^(\\d{2}:\\d{2})~(\\d{2}:\\d{2})(.+)$");
    std::smatch match;
    if (!std::regex_match(line, match, activity_regex)) {
        errors.insert({line_num, "Activity line format error. Expected 'HH:MM~HH:MMTextContent'.", ErrorType::LineFormat});
        return;
    }

    std::string start_time_str = match[1].str();
    std::string end_time_str = match[2].str();
    std::string activity_text = trim(match[3].str());

    int start_hh, start_mm, end_hh, end_mm;
    bool start_valid = parse_time_format(start_time_str, start_hh, start_mm);
    bool end_valid = parse_time_format(end_time_str, end_hh, end_mm);

    if (!start_valid) errors.insert({line_num, "Activity start time invalid.", ErrorType::LineFormat});
    if (!end_valid) errors.insert({line_num, "Activity end time invalid.", ErrorType::LineFormat});

    if (start_valid && end_valid) {
        if (!block.last_activity_end_time.empty() && block.last_activity_end_time != start_time_str) {
            errors.insert({line_num, "Previous activity ended at " + block.last_activity_end_time + " but this one starts at " + start_time_str + ".", ErrorType::TimeDiscontinuity});
        }
        block.last_activity_end_time = end_time_str;
        if (start_hh == end_hh && end_mm < start_mm) {
            errors.insert({line_num, "Activity end time cannot be before start time on the same hour.", ErrorType::LineFormat});
        }
    }

    static const std::regex text_content_regex("^[a-zA-Z0-9_-]+$");
    if (!std::regex_match(activity_text, text_content_regex)) {
        errors.insert({line_num, "Activity text '" + activity_text + "' contains invalid characters.", ErrorType::Logical});
        return;
    }
    if (activity_text.find('_') == std::string::npos) {
        errors.insert({line_num, "Activity text '" + activity_text + "' must contain an underscore.", ErrorType::Logical});
        return;
    }
    if (activity_text.back() == '_') {
        errors.insert({line_num, "Activity text '" + activity_text + "' cannot end with an underscore.", ErrorType::Logical});
        return;
    }

    if (config_.loaded) {
        if (config_.parent_categories.count(activity_text)) {
            errors.insert({line_num, "Activity '" + activity_text + "' cannot be a parent category.", ErrorType::Logical});
            return;
        }
        bool prefix_matched = false;
        for (const auto& pair : config_.parent_categories) {
            std::string prefix = pair.first + "_";
            if (activity_text.rfind(prefix, 0) == 0) {
                prefix_matched = true;
                if (pair.second.find(activity_text) == pair.second.end()) {
                    errors.insert({line_num, "Activity '" + activity_text + "' is not an allowed sub-tag for parent '" + pair.first + "'.", ErrorType::Logical});
                }
                break;
            }
        }
        if (!prefix_matched) {
            errors.insert({line_num, "Activity '" + activity_text + "' does not match any defined parent category prefix.", ErrorType::Logical});
        }
    }
    
    block.activity_lines_content.push_back({activity_text, line_num});
}

void FormatValidator::finalize_block_status_validation(DateBlock& block, std::set<Error>& errors) {
    if (!block.status_format_valid) return;

    bool contains_study = false;
    for (const auto& activity_pair : block.activity_lines_content) {
        if (activity_pair.first.find("study") != std::string::npos) {
            contains_study = true;
            break;
        }
    }

    if (contains_study && !block.status_value_from_file) {
        errors.insert({block.status_line_num, "Status must be 'True' because a 'study' activity was found.", ErrorType::Logical});
    } else if (!contains_study && block.status_value_from_file) {
        errors.insert({block.status_line_num, "Status must be 'False' because no 'study' activity was found.", ErrorType::Logical});
    }
}