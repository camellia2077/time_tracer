#include "FormatValidator.h"
#include "SharedUtils.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <regex>
#include <algorithm>
#include <vector>

// --- Internal Data Structures ---

// Structure to hold data for a single date block during parsing
struct FormatValidator::DateBlock {
    int start_line_number = -1;
    std::string date_str;
    int date_line_num = -1;

    bool status_value_from_file = false;
    bool status_format_valid = false;
    int status_line_num = -1;
    
    std::vector<std::pair<std::string, int>> activity_lines_content;
    bool header_completely_valid = true;

    void reset() {
        start_line_number = -1;
        date_str.clear();
        date_line_num = -1;
        status_value_from_file = false;
        status_format_valid = false;
        status_line_num = -1;
        activity_lines_content.clear();
        header_completely_valid = true;
    }
};

// --- Constructor ---
FormatValidator::FormatValidator(const std::string& config_filename)
    : config_filepath_(config_filename) {
    loadConfiguration();
    if (config_.loaded) {
        std::cout << GREEN_COLOR << "Format validator configuration loaded successfully." << RESET_COLOR << std::endl;
    } else {
        std::cout << YELLOW_COLOR << "Warning: Format validator running with limited validation due to configuration issues." << RESET_COLOR << std::endl;
    }
}

// --- Public Methods ---
bool FormatValidator::validateFile(const std::string& file_path, std::set<Error>& errors) {
    std::ifstream file(file_path);
    if (!file.is_open()) {
        errors.insert({0, "Could not open file: " + file_path});
        return false;
    }

    std::string line;
    int line_number = 0;
    
    DateBlock current_block;
    bool first_non_empty_line = true;
    bool date_line_encountered = false;
    
    enum class ParseState { EXPECT_DATE, EXPECT_STATUS, EXPECT_GETUP, EXPECT_REMARK, EXPECT_ACTIVITY };
    ParseState state = ParseState::EXPECT_DATE;

    auto finalize_previous_block = [&](DateBlock& block) {
        if (block.date_line_num != -1) {
            if (block.header_completely_valid) {
                finalize_block_status_validation(block, errors);
            }
            block.reset();
        }
    };

    while (std::getline(file, line)) {
        line_number++;
        std::string trimmed_line = trim(line);

        if (first_non_empty_line && !trimmed_line.empty()) {
            first_non_empty_line = false;
            if (trimmed_line.rfind("Date:", 0) != 0) {
                errors.insert({line_number, "File must begin with a valid Date: line."});
                current_block.header_completely_valid = false;
            }
        }
        
        if (trimmed_line.empty()) continue;

        if (trimmed_line.rfind("Date:", 0) == 0) {
            if (date_line_encountered) {
                 if (state != ParseState::EXPECT_ACTIVITY && state != ParseState::EXPECT_DATE) {
                     errors.insert({current_block.start_line_number, "New Date: line found before previous block was complete."});
                 }
                 finalize_previous_block(current_block);
            }
            date_line_encountered = true;
            current_block.start_line_number = line_number;
            validate_date_line(trimmed_line, line_number, current_block, errors);
            state = current_block.header_completely_valid ? ParseState::EXPECT_STATUS : ParseState::EXPECT_DATE;
        } else {
            if (!date_line_encountered) {
                errors.insert({line_number, "Expected Date: line to start a block."});
                state = ParseState::EXPECT_DATE;
                continue;
            }
            if (!current_block.header_completely_valid) continue;

            switch (state) {
                case ParseState::EXPECT_STATUS:
                    validate_status_line(trimmed_line, line_number, current_block, errors);
                    state = ParseState::EXPECT_GETUP;
                    break;
                case ParseState::EXPECT_GETUP:
                    validate_getup_line(trimmed_line, line_number, current_block, errors);
                    state = ParseState::EXPECT_REMARK;
                    break;
                case ParseState::EXPECT_REMARK:
                    validate_remark_line(trimmed_line, line_number, current_block, errors);
                    state = ParseState::EXPECT_ACTIVITY;
                    break;
                case ParseState::EXPECT_ACTIVITY:
                    validate_activity_line(trimmed_line, line_number, current_block, errors);
                    break;
                case ParseState::EXPECT_DATE: // In an error state, wait for the next Date
                    break;
            }
        }
    }

    if (date_line_encountered) {
        if (state != ParseState::EXPECT_ACTIVITY && state != ParseState::EXPECT_DATE) {
             errors.insert({current_block.start_line_number, "File ended before final block was complete."});
        }
       finalize_previous_block(current_block);
    } else if (!first_non_empty_line) {
        errors.insert({1, "File contains text but no Date: line was found."});
    }

    return errors.empty();
}


// --- Private Helper Methods ---

void FormatValidator::loadConfiguration() {
    std::ifstream ifs(config_filepath_);
    if (!ifs.is_open()) {
        std::cerr << YELLOW_COLOR << "Warning: Config file '" << config_filepath_ << "' not found." << RESET_COLOR << std::endl;
        return;
    }
    try {
        nlohmann::json j;
        ifs >> j;
        if (j.contains("PARENT_CATEGORIES") && j["PARENT_CATEGORIES"].is_object()) {
            for (auto const& [key, val] : j["PARENT_CATEGORIES"].items()) {
                if (val.is_array()) {
                    config_.parent_categories[key] = val.get<std::unordered_set<std::string>>();
                }
            }
            config_.loaded = true;
        }
    } catch (const std::exception& e) {
        std::cerr << YELLOW_COLOR << "Warning: Error processing config file '" << config_filepath_ << "': " << e.what() << RESET_COLOR << std::endl;
    }
}

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

void FormatValidator::validate_date_line(const std::string& line, int line_num, DateBlock& block, std::set<Error>& errors) {
    block.date_line_num = line_num;
    static const std::regex date_regex("^Date:(\\d{8})$");
    std::smatch match;
    if (std::regex_match(line, match, date_regex)) {
        block.date_str = match[1].str();
    } else {
        errors.insert({line_num, "Date line format error. Expected 'Date:YYYYMMDD'."});
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
        errors.insert({line_num, "Status line format error. Expected 'Status:True' or 'Status:False'."});
        block.status_format_valid = false;
        block.header_completely_valid = false;
    }
}

void FormatValidator::validate_getup_line(const std::string& line, int line_num, DateBlock& block, std::set<Error>& errors) {
    static const std::regex getup_regex("^Getup:(\\d{2}:\\d{2})$");
    std::smatch match;
    if (std::regex_match(line, match, getup_regex)) {
        int hh, mm;
        if (!parse_time_format(match[1].str(), hh, mm)) {
            errors.insert({line_num, "Getup time value invalid."});
            block.header_completely_valid = false;
        }
    } else {
        errors.insert({line_num, "Getup line format error. Expected 'Getup:HH:MM'."});
        block.header_completely_valid = false;
    }
}

void FormatValidator::validate_remark_line(const std::string& line, int line_num, DateBlock& block, std::set<Error>& errors) {
    if (line.rfind("Remark:", 0) != 0) {
        errors.insert({line_num, "Remark line format error. Expected 'Remark:' prefix."});
        block.header_completely_valid = false;
    }
}

void FormatValidator::validate_activity_line(const std::string& line, int line_num, DateBlock& block, std::set<Error>& errors) {
    static const std::regex activity_regex("^(\\d{2}:\\d{2})~(\\d{2}:\\d{2})(.+)$");
    std::smatch match;
    if (!std::regex_match(line, match, activity_regex)) {
        errors.insert({line_num, "Activity line format error. Expected 'HH:MM~HH:MMTextContent'."});
        return;
    }

    std::string start_time_str = match[1].str();
    std::string end_time_str = match[2].str();
    std::string activity_text = trim(match[3].str());

    int start_hh, start_mm, end_hh, end_mm;
    bool start_valid = parse_time_format(start_time_str, start_hh, start_mm);
    bool end_valid = parse_time_format(end_time_str, end_hh, end_mm);

    if (!start_valid) errors.insert({line_num, "Activity start time invalid."});
    if (!end_valid) errors.insert({line_num, "Activity end time invalid."});
    if (start_valid && end_valid && start_hh == end_hh && end_mm < start_mm) {
        errors.insert({line_num, "Activity end time cannot be before start time on the same hour."});
    }

    // Syntax validation
    static const std::regex text_content_regex("^[a-zA-Z0-9_-]+$");
    if (!std::regex_match(activity_text, text_content_regex)) {
        errors.insert({line_num, "Activity text '" + activity_text + "' contains invalid characters."});
        return;
    }
    if (activity_text.find('_') == std::string::npos) {
        errors.insert({line_num, "Activity text '" + activity_text + "' must contain an underscore."});
        return;
    }
    if (activity_text.back() == '_') {
        errors.insert({line_num, "Activity text '" + activity_text + "' cannot end with an underscore."});
        return;
    }

    // Config validation
    if (config_.loaded) {
        if (config_.parent_categories.count(activity_text)) {
            errors.insert({line_num, "Activity '" + activity_text + "' cannot be a parent category."});
            return;
        }
        bool prefix_matched = false;
        for (const auto& pair : config_.parent_categories) {
            std::string prefix = pair.first + "_";
            if (activity_text.rfind(prefix, 0) == 0) {
                prefix_matched = true;
                if (pair.second.find(activity_text) == pair.second.end()) {
                    errors.insert({line_num, "Activity '" + activity_text + "' is not an allowed sub-tag for parent '" + pair.first + "'."});
                }
                break;
            }
        }
        if (!prefix_matched) {
            errors.insert({line_num, "Activity '" + activity_text + "' does not match any defined parent category prefix."});
        }
    }
    
    // If all checks passed for this line, add it to the block for status validation
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
        errors.insert({block.status_line_num, "Status must be 'True' because a 'study' activity was found."});
    } else if (!contains_study && block.status_value_from_file) {
        errors.insert({block.status_line_num, "Status must be 'False' because no 'study' activity was found."});
    }
}