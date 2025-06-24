// FormatValidator.cpp

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
#include <iomanip> // For std::setw, std::setfill in increment_date

// --- START CORRECTION: ADDING MISSING FUNCTION IMPLEMENTATIONS ---

// --- 修改：更新构造函数实现 ---
// Constructor: Initializes the validator and loads configuration files.
FormatValidator::FormatValidator(const std::string& config_filename, const std::string& header_config_filename, bool enable_day_count_check)
    : config_filepath_(config_filename), 
      header_config_filepath_(header_config_filename),
      check_day_count_enabled_(enable_day_count_check) { // 初始化新成员
    loadConfiguration();
}

// Loads the JSON configuration for the validator.
void FormatValidator::loadConfiguration() {
    // Load main configuration (parent categories)
    std::ifstream config_ifs(config_filepath_);
    if (config_ifs.is_open()) {
        try {
            nlohmann::json j;
            config_ifs >> j;
            
            // --- 修改的部分 ---
            if (j.contains("PARENT_CATEGORIES")) {
                config_.parent_categories = j["PARENT_CATEGORIES"].get<std::map<std::string, std::unordered_set<std::string>>>();
                config_.loaded = true;
            } else {
                 std::cerr << RED_COLOR << "Error: Validator config JSON does not contain 'PARENT_CATEGORIES' key." << RESET_COLOR << std::endl;
                 config_.loaded = false;
            }
            // --- 修改结束 ---

        } catch (const std::exception& e) {
            std::cerr << RED_COLOR << "Error processing validator config JSON: " << e.what() << RESET_COLOR << std::endl;
            config_.loaded = false; // 确保在出错时loaded为false
        }
    } else {
        std::cerr << RED_COLOR << "Error: Could not open validator config file: " << config_filepath_ << RESET_COLOR << std::endl;
    }

    // Load header order configuration (这部分代码是正确的，无需修改)
    std::ifstream header_ifs(header_config_filepath_);
    if (header_ifs.is_open()) {
        try {
            nlohmann::json j;
            header_ifs >> j;
            if (j.contains("header_order") && j["header_order"].is_array()) {
                header_order_ = j["header_order"].get<std::vector<std::string>>();
            }
        } catch (const std::exception& e) {
            std::cerr << RED_COLOR << "Error processing header format JSON: " << e.what() << RESET_COLOR << std::endl;
        }
    } else {
        std::cerr << RED_COLOR << "Error: Could not open header format file: " << header_config_filepath_ << RESET_COLOR << std::endl;
    }
}

// Utility function to trim whitespace from both ends of a string.
std::string FormatValidator::trim(const std::string& str) {
    const std::string WHITESPACE = " \n\r\t\f\v";
    size_t first = str.find_first_not_of(WHITESPACE);
    if (std::string::npos == first) {
        return "";
    }
    size_t last = str.find_last_not_of(WHITESPACE);
    return str.substr(first, (last - first + 1));
}

// Parses a time string "HH:MM" into integer hours and minutes.
bool FormatValidator::parse_time_format(const std::string& time_str, int& hours, int& minutes) {
    static const std::regex time_regex("^([0-1]?[0-9]|2[0-3]):([0-5][0-9])$");
    std::smatch match;
    if (std::regex_match(time_str, match, time_regex)) {
        hours = std::stoi(match[1].str());
        minutes = std::stoi(match[2].str());
        return true;
    }
    return false;
}

// Placeholder implementations for other validation functions.
// You can expand these with more specific logic if needed.
void FormatValidator::validate_status_line(const std::string& line, int line_num, DateBlock& block, std::set<Error>& errors) { /* Add specific logic if needed */ }
void FormatValidator::validate_sleep_line(const std::string& line, int line_num, DateBlock& block, std::set<Error>& errors) { /* Add specific logic if needed */ }
void FormatValidator::validate_getup_line(const std::string& line, int line_num, DateBlock& block, std::set<Error>& errors) { /* Add specific logic if needed */ }
void FormatValidator::validate_remark_line(const std::string& line, int line_num, DateBlock& block, std::set<Error>& errors) { /* Add specific logic if needed */ }
void FormatValidator::validate_activity_line(const std::string& line, int line_num, DateBlock& block, std::set<Error>& errors) { /* Add specific logic if needed */ }

// This function performs final checks on a completed block, like time continuity.
void FormatValidator::finalize_block_status_validation(DateBlock& block, std::set<Error>& errors) {
    // This is an example of a check this function might perform (time continuity).
    std::string last_time_str = ""; // Assume starting with getup time if available
    // You would need to parse Getup time and store it in the block to use it here.
    // For now, we'll just check activity continuity.
    for (const auto& activity : block.activity_lines_content) {
        // Logic to check for time gaps/overlaps between activities would go here.
    }
}

// --- END CORRECTION ---


// validateFile is modified to orchestrate the new checks.
bool FormatValidator::validateFile(const std::string& file_path, std::set<Error>& errors) {
    std::ifstream file(file_path);
    if (!file.is_open()) {
        errors.insert({0, "Could not open file: " + file_path, ErrorType::FileAccess});
        return false;
    }

    std::string line;
    int line_number = 0;
    DateBlock current_block; // This now compiles correctly
    
    size_t expected_header_idx = 0;
    bool in_activity_section = false;
    
    // --- MODIFICATION: Clear collected dates vector for new file ---
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
                // This logic needs refinement based on actual file structure
                // Assuming Remark is the last header before activities start
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
                // In a complete system, you'd call other validate_* functions here
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

    // --- MODIFICATION: Perform date checks after parsing the whole file ---
    validate_date_continuity(errors);
    // --- MODIFICATION: Add call to the new month start validation function ---
    validate_month_start(month_day_map, errors);
    validate_all_days_for_month(month_day_map, errors);

    return errors.empty();
}

// New helper function implementations
bool FormatValidator::is_leap(int year) {
    return (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0));
}

int FormatValidator::days_in_month(int year, int month) {
    if (month == 2) {
        return is_leap(year) ? 29 : 28;
    } else if (month == 4 || month == 6 || month == 9 || month == 11) {
        return 30;
    } else {
        return 31;
    }
}

std::string FormatValidator::increment_date(const std::string& date_str) {
    if (date_str.length() != 8) return ""; // Invalid format

    int year = std::stoi(date_str.substr(0, 4));
    int month = std::stoi(date_str.substr(4, 2));
    int day = std::stoi(date_str.substr(6, 2));

    day++;
    if (day > days_in_month(year, month)) {
        day = 1;
        month++;
        if (month > 12) {
            month = 1;
            year++;
        }
    }

    std::stringstream ss;
    ss << year;
    ss << std::setw(2) << std::setfill('0') << month;
    ss << std::setw(2) << std::setfill('0') << day;
    return ss.str();
}

// --- MODIFICATION: Date continuity check is removed. This function now only parses and collects dates. ---
void FormatValidator::validate_date_line(const std::string& line, int line_num, DateBlock& block, std::set<Error>& errors) {
    block.date_line_num = line_num;
    static const std::regex date_regex("^Date:(\\d{8})$");
    std::smatch match;
    if (std::regex_match(line, match, date_regex)) {
        block.date_str = match[1].str();
        // Collect the date and its line number for later validation.
        collected_dates_.push_back({block.date_str, line_num});
    } else {
        errors.insert({line_num, "Date line format error. Expected 'Date:YYYYMMDD'.", ErrorType::LineFormat});
        block.header_completely_valid = false;
    }
}

// --- MODIFICATION: New function to check continuity on all collected dates except the last one. ---
void FormatValidator::validate_date_continuity(std::set<Error>& errors) {
    // There must be at least two dates to check for continuity.
    if (collected_dates_.size() < 2) {
        return;
    }

    // Iterate up to the second-to-last date. This avoids checking continuity for the final date entry.
    for (size_t i = 0; i < collected_dates_.size() - 1; ++i) {
        const auto& current_date_info = collected_dates_[i];
        const auto& next_date_info = collected_dates_[i+1];

        std::string expected_date = increment_date(current_date_info.first);

        if (next_date_info.first != expected_date) {
            errors.insert({next_date_info.second, "Date is not consecutive. Expected " + expected_date + " after " + current_date_info.first + ".", ErrorType::DateContinuity});
        }
    }
}

// --- MODIFICATION: Added new function implementation ---
void FormatValidator::validate_month_start(const std::map<std::string, std::set<int>>& month_day_map, std::set<Error>& errors) {
    for (const auto& pair : month_day_map) {
        const std::string& yyyymm = pair.first;
        const std::set<int>& days = pair.second;

        if (days.empty()) {
            continue; // Should not happen, but a safe check
        }

        // For any given month, the first day recorded must be 1.
        // The `std::set` keeps the days ordered, so we check the first element.
        if (*days.begin() != 1) {
            std::string year = yyyymm.substr(0, 4);
            std::string month_str = yyyymm.substr(4, 2);
            // This is a file-level structural error, so line number 0 is appropriate.
            errors.insert({0, "Data for month " + year + "-" + month_str + " must start from day 01.", ErrorType::Structural});
        }
    }
}


// --- MODIFICATION: This function is completely replaced with the enhanced version ---
void FormatValidator::validate_all_days_for_month(const std::map<std::string, std::set<int>>& month_day_map, std::set<Error>& errors) {
    // If the check is not enabled via the command line, do nothing.
    if (!check_day_count_enabled_) {
        return;
    }

    for (const auto& pair : month_day_map) {
        const std::string& yyyymm = pair.first;
        const std::set<int>& days = pair.second;

        int year = std::stoi(yyyymm.substr(0, 4));
        int month = std::stoi(yyyymm.substr(4, 2));

        int expected_days = days_in_month(year, month);
        
        if (days.size() != expected_days) {
            // Find which days are missing
            std::vector<int> missing_days_vec;
            for (int d = 1; d <= expected_days; ++d) {
                if (days.find(d) == days.end()) {
                    missing_days_vec.push_back(d);
                }
            }

            // Build the detailed, multi-line error message
            std::stringstream msg_ss;
            msg_ss << "Incorrect day count for " << yyyymm.substr(0, 4) << "-" << yyyymm.substr(4, 2)
                   << ". Expected " << expected_days << " days, but found " << days.size() << ".";

            // Add the list of specific missing days
            if (!missing_days_vec.empty()) {
                msg_ss << "\n  Missing days: ";
                for (size_t i = 0; i < missing_days_vec.size(); ++i) {
                    msg_ss << missing_days_vec[i] << (i == missing_days_vec.size() - 1 ? "" : ", ");
                }
            }

            // Add the color-coded month overview
            msg_ss << "\n  Month overview: [";
            for (int d = 1; d <= expected_days; ++d) {
                bool is_present = (days.find(d) != days.end());
                if (is_present) {
                    msg_ss << GREEN_COLOR << d << RESET_COLOR;
                } else {
                    msg_ss << RED_COLOR << d << RESET_COLOR;
                }
                if (d < expected_days) {
                    msg_ss << ", ";
                }
            }
            msg_ss << "]";

            // Insert the complete message as a single error
            errors.insert({0, msg_ss.str(), ErrorType::IncorrectDayCountForMonth});
        }
    }
}