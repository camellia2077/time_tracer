// reprocessing/validator/source_txt/SourceFileValidator.cpp
#include "SourceFileValidator.hpp"
#include "common/AnsiColors.hpp" 
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

// --- [核心修改] 更新整个 validate 方法以适应新的多年份格式 ---
bool SourceFileValidator::validate(const std::string& file_path, std::set<Error>& errors) {
    std::ifstream inFile(file_path);
    if (!inFile.is_open()) {
        errors.insert({0, "Could not open file: " + file_path, ErrorType::FileAccess});
        return false;
    }

    std::string line;
    int lineNumber = 0;
    
    // --- 新的状态变量 ---
    std::string currentYear = "";               // 当前正在处理的年份
    bool dateFoundInBlock = false;          // 在当前的年份区块中是否已找到日期
    bool eventFoundForCurrentDay = false;   // 当天是否已出现过事件行

    while (std::getline(inFile, line)) {
        lineNumber++;
        std::string trimmed_line = trim(line);
        if (trimmed_line.empty()) continue;

        // 1. 检查是否为年份行
        if (isYearLine(trimmed_line)) {
            currentYear = trimmed_line.substr(1);
            dateFoundInBlock = false;       // 重置状态，因为进入了新的年份区块
            eventFoundForCurrentDay = false;
            continue;
        }

        // 2. 在找到任何年份行之前，不允许出现其他内容
        if (currentYear.empty()) {
            errors.insert({lineNumber, "The file must start with a year header (e.g., 'y2025'). Found: '" + trimmed_line + "'", ErrorType::Source_MissingYearHeader});
            // 这是一个严重的结构性错误，后续的验证没有意义
            return false;
        }

        // 3. 检查是否为日期行
        if (isDateLine(trimmed_line)) {
            dateFoundInBlock = true;
            eventFoundForCurrentDay = false; // 新的一天开始，重置事件标志
            continue;
        }

        // 4. 在当前年份区块找到任何日期行之前，不允许出现其他内容
        if (!dateFoundInBlock) {
            errors.insert({lineNumber, "A 4-digit date (MMDD) must follow the year header. Found: '" + trimmed_line + "'", ErrorType::Source_NoDateAtStart});
            continue;
        }

        // 5. 检查是否为备注行
        if (isRemarkLine(trimmed_line)) {
            if (eventFoundForCurrentDay) {
                errors.insert({lineNumber, "Remark lines cannot appear after an event line for the same day. Found: '" + trimmed_line + "'", ErrorType::Source_RemarkAfterEvent});
            }
            continue;
        }
        
        // 6. 尝试作为事件行进行解析和验证
        if (parseAndValidateEventLine(trimmed_line, errors, lineNumber, !eventFoundForCurrentDay)) {
            eventFoundForCurrentDay = true;
            continue;
        }

        // 7. 如果以上都不是，则为无效行格式
        errors.insert({lineNumber, "Invalid format. Must be a year (yYYYY), date (MMDD), remark (r text), or a valid event (HHMMdesc). Found: '" + trimmed_line + "'", ErrorType::Source_InvalidLineFormat});
    }

    return errors.empty();
}

bool SourceFileValidator::isYearLine(const std::string& line) {
    if (line.length() != 5 || line[0] != 'y') {
        return false;
    }
    return std::all_of(line.begin() + 1, line.end(), ::isdigit);
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

        if (is_first_event) {
            if (wake_keywords_.count(description) == 0) {
                 errors.insert({line_number, "Unrecognized wake-up activity '" + description + "'. The first activity of the day must be a valid wake keyword (e.g., '起床').", ErrorType::UnrecognizedActivity});
            }
        } else {
            if (valid_event_keywords_.count(description) == 0) {
                 errors.insert({line_number, "Unrecognized activity '" + description + "'. Please check spelling or update config file.", ErrorType::UnrecognizedActivity});
            }
        }
        return true; 
    } catch (const std::exception&) {
        return false;
    }
}