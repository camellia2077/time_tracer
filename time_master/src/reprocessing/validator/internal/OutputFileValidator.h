// reprocessing/validator/internal/OutputFileValidator.h
#ifndef OUTPUT_FILE_VALIDATOR_H
#define OUTPUT_FILE_VALIDATOR_H

#include "reprocessing/validator/ValidatorUtils.h"
#include <string>
#include <vector>
#include <set>
#include <unordered_set>
#include <map>
#include <nlohmann/json.hpp>

class OutputFileValidator {
public:
    OutputFileValidator(const std::string& validator_config_path, const std::string& header_config_path, bool enable_day_count_check = false);

    bool validate(const std::string& file_path, std::set<Error>& errors);

private:
    // 内部数据结构，用于存储一个日期块的状态
    struct DateBlock {
        int start_line_number = -1;
        int date_line_num = -1;
        std::string date_str;
        bool header_completely_valid = true;
        bool sleep_value_from_file = false;
        std::vector<std::pair<std::string, int>> activity_lines_content;
        std::string last_activity_end_time;
        
        void reset();
    };
    
    // 配置信息
    std::string validator_config_path_;
    std::string header_config_path_;
    std::map<std::string, std::unordered_set<std::string>> parent_categories_;
    std::vector<std::string> header_order_;
    bool config_loaded_ = false;
    bool check_day_count_enabled_;

    // 私有辅助函数
    void loadConfiguration();
    bool is_valid_activity(const std::string& activity_name) const;
    bool parse_time_format(const std::string& time_str, int& hours, int& minutes);
    
    // 验证流程的子函数
    void validate_activity_line(const std::string& line, int line_num, DateBlock& block, std::set<Error>& errors);
    void finalize_block_validation(DateBlock& block, std::set<Error>& errors);
    
    // 日期连续性检查函数
    void validate_date_logic(const std::string& file_path, std::set<Error>& errors);
};

#endif // OUTPUT_FILE_VALIDATOR_H