#include "OutputFileValidator.h"
#include "common_utils.h"
#include <fstream>
#include <iostream>
#include <algorithm>

OutputFileValidator::OutputFileValidator(const std::string& validator_config_path, const std::string& header_config_path, bool enable_day_count_check)
    : validator_config_path_(validator_config_path),
      header_config_path_(header_config_path),
      check_day_count_enabled_(enable_day_count_check) {
    loadConfiguration();
}

void OutputFileValidator::loadConfiguration() {
    std::ifstream v_ifs(validator_config_path_);
    if (v_ifs.is_open()) {
        try {
            nlohmann::json j;
            v_ifs >> j;
            if (j.contains("PARENT_CATEGORIES")) {
                parent_categories_ = j["PARENT_CATEGORIES"].get<std::map<std::string, std::unordered_set<std::string>>>();
                config_loaded_ = true;
            }
        } catch (...) { /* 错误已在外部处理或忽略 */ }
    }

    std::ifstream h_ifs(header_config_path_);
    if (h_ifs.is_open()) {
        try {
            nlohmann::json j;
            h_ifs >> j;
            if (j.contains("header_order")) {
                header_order_ = j["header_order"].get<std::vector<std::string>>();
            }
        } catch (...) { /* 错误已在外部处理或忽略 */ }
    }
}

bool OutputFileValidator::validate(const std::string& file_path, std::set<Error>& errors) {
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

    auto finalize_and_reset_block = [&](DateBlock& block) {
        if (block.date_line_num != -1) {
            finalize_block_validation(block, errors);
            block.reset();
        }
    };

    while (std::getline(file, line)) {
        line_number++;
        std::string trimmed_line = trim(line);
        if (trimmed_line.empty()) continue;

        if (trimmed_line.rfind("Date:", 0) == 0) {
            finalize_and_reset_block(current_block);
            current_block.start_line_number = line_number;
            in_activity_section = false;
            expected_header_idx = 0;
        }

        if (current_block.start_line_number == -1) {
            errors.insert({line_number, "File must begin with a valid Date: line.", ErrorType::Structural});
            continue;
        }

        if (!current_block.header_completely_valid) continue;

        if (!in_activity_section) {
             if (expected_header_idx >= header_order_.size()) {
                 errors.insert({line_number, "Unexpected line after header section.", ErrorType::Structural});
                 current_block.header_completely_valid = false;
                 continue;
             }
            const std::string& expected_header = header_order_[expected_header_idx];
            if (trimmed_line.rfind(expected_header, 0) == 0) {
                 if(expected_header == "Remark:") {
                     in_activity_section = true;
                     // 如果 Remark: 行本身也包含了内容，它不是活动，所以直接跳过
                 } else if (expected_header == "Sleep:") {
                     if (trimmed_line == "Sleep:True") current_block.sleep_value_from_file = true;
                 } else if (expected_header == "Date:") {
                     current_block.date_line_num = line_number;
                     current_block.date_str = trimmed_line.substr(5);
                 }
                expected_header_idx++;
            } else {
                 errors.insert({line_number, "Line out of order. Expected '" + expected_header + "'.", ErrorType::Structural});
                 current_block.header_completely_valid = false;
            }
        } else {
            // 在 Remark: 之后的所有非空行都应是活动行
            validate_activity_line(trimmed_line, line_number, current_block, errors);
        }
    }
    finalize_and_reset_block(current_block);

    if (check_day_count_enabled_) {
        validate_date_logic(file_path, errors);
    }
    
    return errors.empty();
}

void OutputFileValidator::validate_activity_line(const std::string& line, int line_num, DateBlock& block, std::set<Error>& errors) {
    size_t tilde_pos = line.find('~');
    if (tilde_pos == std::string::npos || tilde_pos == 0) {
        errors.insert({line_num, "Invalid activity format. Expected 'HH:MM~HH:MMactivity'.", ErrorType::LineFormat});
        return;
    }
    std::string start_time_str = line.substr(0, tilde_pos);
    std::string rest = line.substr(tilde_pos + 1);
    if (rest.length() < 6) { // 至少需要 "HH:MMa"
        errors.insert({line_num, "Invalid activity format after '~'. Expected 'HH:MMactivity'.", ErrorType::LineFormat});
        return;
    }
    std::string end_time_str = rest.substr(0, 5);
    std::string activity_name = rest.substr(5);

    int start_h, start_m, end_h, end_m;
    if (!parse_time_format(start_time_str, start_h, start_m) || !parse_time_format(end_time_str, end_h, end_m)) {
        errors.insert({line_num, "Invalid time format in activity line.", ErrorType::LineFormat});
        return;
    }

    if (!block.last_activity_end_time.empty() && start_time_str != block.last_activity_end_time) {
        errors.insert({line_num, "Time discontinuity. Expected start time " + block.last_activity_end_time + ", but got " + start_time_str + ".", ErrorType::TimeDiscontinuity});
    }

    if (!is_valid_activity(activity_name)) {
        errors.insert({line_num, "Unknown activity '" + activity_name + "'.", ErrorType::Logical});
    }
    
    block.activity_lines_content.push_back({activity_name, line_num});
    block.last_activity_end_time = end_time_str;
}

void OutputFileValidator::finalize_block_validation(DateBlock& block, std::set<Error>& errors) {
    if (!block.header_completely_valid) return;

    if (block.sleep_value_from_file && !block.activity_lines_content.empty()) {
        const auto& last_activity = block.activity_lines_content.back();
        if (last_activity.first != "sleep_night") {
            errors.insert({last_activity.second, "The last activity must be 'sleep_night' when Sleep is True.", ErrorType::MissingSleepNight});
        }
    }
}

void OutputFileValidator::DateBlock::reset() {
    start_line_number = -1; date_line_num = -1; date_str.clear();
    header_completely_valid = true; sleep_value_from_file = false;
    activity_lines_content.clear(); last_activity_end_time.clear();
}

bool OutputFileValidator::parse_time_format(const std::string& time_str, int& hours, int& minutes) {
    if (time_str.length() != 5 || time_str[2] != ':') return false;
    try {
        hours = std::stoi(time_str.substr(0, 2));
        minutes = std::stoi(time_str.substr(3, 2));
        return (hours >= 0 && hours <= 23 && minutes >= 0 && minutes <= 59);
    } catch (...) { return false; }
}

bool OutputFileValidator::is_valid_activity(const std::string& activity_name) const {
    if (!config_loaded_) return true; // 如果配置未加载，则默认所有活动都有效
    for (const auto& parent_pair : parent_categories_) {
        if (parent_pair.second.count(activity_name)) {
            return true;
        }
    }
    return false;
}

void OutputFileValidator::validate_date_logic(const std::string& file_path, std::set<Error>& errors){
    // 日期连续性和完整性检查的逻辑可以放在这里
    // (这部分逻辑较为复杂，作为占位符保留)
}