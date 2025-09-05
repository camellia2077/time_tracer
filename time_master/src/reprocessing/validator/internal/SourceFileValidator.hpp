// reprocessing/validator/internal/SourceFileValidator.hpp
#ifndef SOURCE_FILE_VALIDATOR_HPP
#define SOURCE_FILE_VALIDATOR_HPP

#include "reprocessing/validator/ValidatorUtils.hpp"
#include <string>
#include <set>
#include <unordered_set>
#include <nlohmann/json.hpp>

class SourceFileValidator {
public:
    explicit SourceFileValidator(const std::string& config_filename);
    bool validate(const std::string& file_path, std::set<Error>& errors);

private:
    std::string remark_prefix_;
    std::unordered_set<std::string> valid_event_keywords_;
    std::unordered_set<std::string> wake_keywords_;

    void loadConfiguration(const std::string& config_filename);
    
    
    bool isYearLine(const std::string& line);// 检查年份行的辅助函数
    bool isDateLine(const std::string& line);// 检查日期行的辅助函数
    bool isRemarkLine(const std::string& line); // 检查备注行的辅助函数
    bool parseAndValidateEventLine(const std::string& line, std::set<Error>& errors, int line_number, bool is_first_event);
};

#endif // SOURCE_FILE_VALIDATOR_HPP