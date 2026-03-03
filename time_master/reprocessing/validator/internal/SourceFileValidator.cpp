#include "common/pch.h"
// reprocessing/validator/internal/SourceFileValidator.cpp
#include "SourceFileValidator.h"

#include "common/common_utils.h"
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
        if (j.contains("remark_prefix") && j["remark_prefix"].is_string()) {
            remark_prefix_ = j["remark_prefix"].get<std::string>();
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
            eventFoundForCurrentDay = false; // 新的一天，重置标志
            if (!firstLineFound) firstLineFound = true;
            continue;
        }

        if (!firstLineFound) {
            errors.insert({lineNumber, "The first non-empty line must be a 4-digit date. Found: '" + trimmed_line + "'", ErrorType::Source_NoDateAtStart});
            continue; // 继续检查，以便发现所有格式问题
        }

        if (isRemarkLine(trimmed_line)) {
            if (eventFoundForCurrentDay) {
                errors.insert({lineNumber, "Remark lines cannot appear after an event line for the same day. Found: '" + trimmed_line + "'", ErrorType::Source_RemarkAfterEvent});
            }
            continue;
        }

        std::string timeStr, description;
        if (parseEventLine(trimmed_line, timeStr, description)) {
            eventFoundForCurrentDay = true;
            continue;
        }

        // 如果以上都不是，则格式错误
        errors.insert({lineNumber, "Invalid format. Must be a date (MMDD), remark (e.g., 'r text'), or event (e.g., '0830event'). Found: '" + trimmed_line + "'", ErrorType::Source_InvalidLineFormat});
    }

    return errors.empty();
}

// --- 私有辅助函数实现 ---

bool SourceFileValidator::isDateLine(const std::string& line) {
    return line.length() == 4 && std::all_of(line.begin(), line.end(), ::isdigit);
}

bool SourceFileValidator::isRemarkLine(const std::string& line) {
    if (remark_prefix_.empty() || line.rfind(remark_prefix_, 0) != 0) {
        return false;
    }
    return !trim(line.substr(remark_prefix_.length())).empty();
}

bool SourceFileValidator::parseEventLine(const std::string& line, std::string& outTimeStr, std::string& outDescription) {
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
        return false;
    }
}