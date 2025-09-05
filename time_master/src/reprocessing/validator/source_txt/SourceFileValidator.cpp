// reprocessing/validator/source_txt/SourceFileValidator.cpp
#include "SourceFileValidator.hpp"
#include "common/common_utils.hpp"
#include <fstream>
#include <iostream>
#include <algorithm>

// 构造函数，在创建对象时直接加载配置
SourceFileValidator::SourceFileValidator(const std::string& config_filename) {
    loadConfiguration(config_filename);
}

// 从JSON文件加载配置
void SourceFileValidator::loadConfiguration(const std::string& config_filename) {
    std::ifstream ifs(config_filename);
    if (!ifs.is_open()) {
        std::cerr << RED_COLOR << "Error: Could not open source validator config file: " << config_filename << RESET_COLOR << std::endl;
        return;
    }
    try {
        nlohmann::json j;
        ifs >> j;
        // 加载备注前缀
        if (j.contains("remark_prefix")) {
            remark_prefix_ = j["remark_prefix"].get<std::string>();
        }
        // 加载有效的事件关键字
        if (j.contains("text_mappings") && j["text_mappings"].is_object()) {
            for (auto& [key, value] : j["text_mappings"].items()) {
                valid_event_keywords_.insert(key);
            }
        }
        // 加载有效的持续时间事件关键字
        if (j.contains("text_duration_mappings") && j["text_duration_mappings"].is_object()) {
            for (auto& [key, value] : j["text_duration_mappings"].items()) {
                valid_event_keywords_.insert(key);
            }
        }
        // 加载唤醒关键字
        if (j.contains("wake_keywords") && j["wake_keywords"].is_array()) {
            for (const auto& keyword : j["wake_keywords"]) {
                wake_keywords_.insert(keyword.get<std::string>());
            }
        }
    } catch (const std::exception& e) {
        std::cerr << RED_COLOR << "Error processing source validator config JSON: " << e.what() << RESET_COLOR << std::endl;
    }
}

// 核心验证逻辑
bool SourceFileValidator::validate(const std::string& file_path, std::set<Error>& errors) {
    std::ifstream inFile(file_path);
    if (!inFile.is_open()) {
        errors.insert({0, "Could not open file: " + file_path, ErrorType::FileAccess});
        return false;
    }

    std::string line;
    int lineNumber = 0;
    // 状态标志
    bool yearLineFound = false;       // 是否已找到年份行
    bool dateLineFound = false;       // 是否已找到第一个日期行
    bool eventFoundForCurrentDay = false; // 当天是否已出现过事件行

    // 逐行读取和验证
    while (std::getline(inFile, line)) {
        lineNumber++;
        std::string trimmed_line = trim(line);
        if (trimmed_line.empty()) continue; // 跳过空行

        // 1. 检查文件的第一个非空行是否为年份行
        if (!yearLineFound) {
            if (isYearLine(trimmed_line)) {
                yearLineFound = true;
                continue;
            } else {
                errors.insert({lineNumber, "The first non-empty line must be a year header (e.g., 'y2025'). Found: '" + trimmed_line + "'", ErrorType::Source_MissingYearHeader});
                return false; // 这是一个严重的结构性错误，直接停止验证
            }
        }

        // 2. 检查是否为日期行
        if (isDateLine(trimmed_line)) {
            eventFoundForCurrentDay = false; // 新的一天开始，重置事件标志
            if (!dateLineFound) dateLineFound = true; // 标记已找到日期行
            continue;
        }

        // 3. 在找到任何日期行之前，不允许出现其他内容
        if (!dateLineFound) {
            errors.insert({lineNumber, "A 4-digit date (MMDD) must follow the year header. Found: '" + trimmed_line + "'", ErrorType::Source_NoDateAtStart});
            continue;
        }

        // 4. 检查是否为备注行
        if (isRemarkLine(trimmed_line)) {
            // 备注行不能出现在当天的事件行之后
            if (eventFoundForCurrentDay) {
                errors.insert({lineNumber, "Remark lines cannot appear after an event line for the same day. Found: '" + trimmed_line + "'", ErrorType::Source_RemarkAfterEvent});
            }
            continue;
        }
        
        // 5. 尝试作为事件行进行解析和验证
        if (parseAndValidateEventLine(trimmed_line, errors, lineNumber, !eventFoundForCurrentDay)) {
            eventFoundForCurrentDay = true; // 标记当天已找到事件
            continue;
        }

        // 6. 如果以上都不是，则为无效行格式
        errors.insert({lineNumber, "Invalid format. Must be a date (MMDD), remark (e.g., 'r text'), or a valid event (e.g., '0830event'). Found: '" + trimmed_line + "'", ErrorType::Source_InvalidLineFormat});
    }

    return errors.empty();
}

// 检查是否为 'y' + 4位数字 的年份行
bool SourceFileValidator::isYearLine(const std::string& line) {
    if (line.length() != 5 || line[0] != 'y') {
        return false;
    }
    // 检查 'y' 后面的四个字符是否都是数字
    return std::all_of(line.begin() + 1, line.end(), ::isdigit);
}

// 检查是否为4位数字的日期行
bool SourceFileValidator::isDateLine(const std::string& line) {
    return line.length() == 4 && std::all_of(line.begin(), line.end(), ::isdigit);
}

// 检查是否为备注行
bool SourceFileValidator::isRemarkLine(const std::string& line) {
    if (remark_prefix_.empty() || line.rfind(remark_prefix_, 0) != 0) return false;
    // 确保备注前缀后有实际内容
    return !trim(line.substr(remark_prefix_.length())).empty();
}

// 解析和验证事件行
bool SourceFileValidator::parseAndValidateEventLine(const std::string& line, std::set<Error>& errors, int line_number, bool is_first_event) {
    // 基本格式检查：长度至少为5 (HHMM + 至少一个字符的描述)，且前4个字符是数字
    if (line.length() < 5 || !std::all_of(line.begin(), line.begin() + 4, ::isdigit)) {
        return false;
    }
    try {
        // 解析时间和描述
        int hh = std::stoi(line.substr(0, 2));
        int mm = std::stoi(line.substr(2, 2));
        if (hh > 23 || mm > 59) return false; // 时间无效
        
        std::string description = line.substr(4);
        if (description.empty()) return false; // 描述不能为空

        // 根据是否为当天的第一个事件，使用不同的关键字集合进行验证
        if (is_first_event) {
            // 如果是第一个事件，必须是有效的“唤醒”活动
            if (wake_keywords_.count(description) == 0) {
                 errors.insert({line_number, "Unrecognized wake-up activity '" + description + "'. The first activity of the day must be a valid wake keyword (e.g., '起床').", ErrorType::UnrecognizedActivity});
            }
        } else {
            // 如果不是第一个事件，必须是通用的有效活动
            if (valid_event_keywords_.count(description) == 0) {
                 errors.insert({line_number, "Unrecognized activity '" + description + "'. Please check spelling or update config file.", ErrorType::UnrecognizedActivity});
            }
        }
        
        // 只要行格式是对的（HHMMdescription），就返回true，即使内容（description）无效
        // 无效内容已经作为Error被记录下来了
        return true; 
    } catch (const std::exception&) {
        // stoi 转换失败等异常
        return false;
    }
}