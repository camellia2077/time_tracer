// reprocessing/validator/source_txt/pipelines/LineProcessor.cpp
#include "LineProcessor.hpp"
// --- [核心修改] 新增头文件引用 ---
#include "common/utils/StringUtils.hpp"
#include <algorithm>

// [修改] 构造函数实现已更新
LineProcessor::LineProcessor(const ConverterConfig& config) : config_(config) {
    // 从 ConverterConfig 中提取并构建验证所需的关键字集合
    const auto& text_map = config.getTextMapping();
    for(const auto& pair : text_map) {
        valid_event_keywords_.insert(pair.first);
    }

    const auto& dur_text_map = config.getTextDurationMapping();
    for(const auto& pair : dur_text_map) {
        valid_event_keywords_.insert(pair.first);
    }

    const auto& wake_vec = config.getWakeKeywords();
    wake_keywords_.insert(wake_vec.begin(), wake_vec.end());
}

bool LineProcessor::is_year(const std::string& line) const {
    if (line.length() != 5 || line[0] != 'y') {
        return false;
    }
    return std::all_of(line.begin() + 1, line.end(), ::isdigit);
}

bool LineProcessor::is_date(const std::string& line) const {
    return line.length() == 4 && std::all_of(line.begin(), line.end(), ::isdigit);
}

bool LineProcessor::is_remark(const std::string& line) const {
    // [修改] 从 ConverterConfig 获取 remark_prefix
    const std::string& prefix = config_.getRemarkPrefix();
    if (prefix.empty() || line.rfind(prefix, 0) != 0) return false;
    return !trim(line.substr(prefix.length())).empty();
}

bool LineProcessor::is_valid_event_line(const std::string& line, int line_number, std::set<Error>& errors) const {
    if (line.length() < 5 || !std::all_of(line.begin(), line.begin() + 4, ::isdigit)) {
        return false;
    }
    try {
        int hh = std::stoi(line.substr(0, 2));
        int mm = std::stoi(line.substr(2, 2));
        if (hh > 23 || mm > 59) return false;
        
        std::string remaining_line = line.substr(4);
        std::string description;
        
        size_t comment_pos = std::string::npos;
        const char* delimiters[] = {"//", "#", ";"};
        for (const char* delim : delimiters) {
            size_t pos = remaining_line.find(delim);
            if (pos != std::string::npos && (comment_pos == std::string::npos || pos < comment_pos)) {
                comment_pos = pos;
            }
        }
        
        description = trim(remaining_line.substr(0, comment_pos));
        if (description.empty()) return false;

        // [修改] 使用内部构建好的关键字集合进行验证
        if (wake_keywords_.count(description) == 0 && valid_event_keywords_.count(description) == 0) {
             errors.insert({line_number, "Unrecognized activity '" + description + "'. Please check spelling or update config file.", ErrorType::UnrecognizedActivity});
        }
        return true; 
    } catch (const std::exception&) {
        return false;
    }
}