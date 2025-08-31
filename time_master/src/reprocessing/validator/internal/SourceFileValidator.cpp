// reprocessing/validator/internal/SourceFileValidator.cpp
#include "SourceFileValidator.hpp"
#include "common/common_utils.hpp"
#include <fstream>
#include <iostream>
#include <algorithm>

SourceFileValidator::SourceFileValidator(const std::string& config_filename) {
    loadConfiguration(config_filename);
}

void SourceFileValidator::loadConfiguration(const std::string& config_filename) {
    std::ifstream ifs(config_filename);
    if (!ifs.is_open()) {
        std::cerr << RED_COLOR << "Error: Could not open source validator config file: " << config_filename << RESET_COLOR << std::endl;
        return;
    }
    try {
        nlohmann::json j;
        ifs >> j;
        if (j.contains("remark_prefix")) {
            remark_prefix_ = j["remark_prefix"].get<std::string>();
        }
        if (j.contains("text_mappings") && j["text_mappings"].is_object()) {
            for (auto& [key, value] : j["text_mappings"].items()) {
                valid_event_keywords_.insert(key);
            }
        }
        if (j.contains("text_duration_mappings") && j["text_duration_mappings"].is_object()) {
            for (auto& [key, value] : j["text_duration_mappings"].items()) {
                valid_event_keywords_.insert(key);
            }
        }
        if (j.contains("wake_keywords") && j["wake_keywords"].is_array()) {
            for (const auto& keyword : j["wake_keywords"]) {
                wake_keywords_.insert(keyword.get<std::string>());
            }
        }
    } catch (const std::exception& e) {
        std::cerr << RED_COLOR << "Error processing source validator config JSON: " << e.what() << RESET_COLOR << std::endl;
    }
}

bool SourceFileValidator::validate(const std::string& file_path, std::set<Error>& errors) {
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

        if (isDateLine(trimmed_line)) {
            eventFoundForCurrentDay = false;
            if (!firstLineFound) firstLineFound = true;
            continue;
        }

        if (!firstLineFound) {
            errors.insert({lineNumber, "The first non-empty line must be a 4-digit date. Found: '" + trimmed_line + "'", ErrorType::Source_NoDateAtStart});
            continue;
        }

        if (isRemarkLine(trimmed_line)) {
            if (eventFoundForCurrentDay) {
                errors.insert({lineNumber, "Remark lines cannot appear after an event line for the same day. Found: '" + trimmed_line + "'", ErrorType::Source_RemarkAfterEvent});
            }
            continue;
        }
        
        if (parseAndValidateEventLine(trimmed_line, errors, lineNumber, !eventFoundForCurrentDay)) {
            eventFoundForCurrentDay = true;
            continue;
        }

        errors.insert({lineNumber, "Invalid format. Must be a date (MMDD), remark (e.g., 'r text'), or a valid event (e.g., '0830event'). Found: '" + trimmed_line + "'", ErrorType::Source_InvalidLineFormat});
    }

    return errors.empty();
}

bool SourceFileValidator::isDateLine(const std::string& line) {
    return line.length() == 4 && std::all_of(line.begin(), line.end(), ::isdigit);
}

bool SourceFileValidator::isRemarkLine(const std::string& line) {
    if (remark_prefix_.empty() || line.rfind(remark_prefix_, 0) != 0) return false;
    return !trim(line.substr(remark_prefix_.length())).empty();
}

bool SourceFileValidator::parseAndValidateEventLine(const std::string& line, std::set<Error>& errors, int line_number, bool is_first_event) {
    if (line.length() < 5 || !std::all_of(line.begin(), line.begin() + 4, ::isdigit)) {
        return false;
    }
    try {
        int hh = std::stoi(line.substr(0, 2));
        int mm = std::stoi(line.substr(2, 2));
        if (hh > 23 || mm > 59) return false;
        
        std::string description = line.substr(4);
        if (description.empty()) return false;

        // [修复] 移除了未使用的 is_valid 变量
        if (is_first_event) {
            // 如果是当天的第一个事件，使用 wake_keywords_ 集合验证
            if (wake_keywords_.count(description) == 0) { // count() 返回0表示未找到
                 errors.insert({line_number, "Unrecognized wake-up activity '" + description + "'. The first activity of the day must be a valid wake keyword (e.g., '起床').", ErrorType::UnrecognizedActivity});
            }
        } else {
            // 如果不是第一个事件，使用 valid_event_keywords_ 集合验证
            if (valid_event_keywords_.count(description) == 0) {
                 errors.insert({line_number, "Unrecognized activity '" + description + "'. Please check spelling or update config file.", ErrorType::UnrecognizedActivity});
            }
        }
        
        return true; // 即使内容无效，但只要行格式是对的（HHMMevent），就返回true
    } catch (const std::exception&) {
        return false;
    }
}