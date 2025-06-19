// FormatValidator.h

#ifndef FORMAT_VALIDATOR_H
#define FORMAT_VALIDATOR_H

#include <string>
#include <vector>
// ... 其他 includes ...
#include <set>
#include <unordered_set>
#include "json.hpp"

class FormatValidator {
public:
    // ... ErrorType 和 Error 结构体保持不变 ...
    enum class ErrorType {
        FileAccess,
        Structural,
        LineFormat,
        TimeDiscontinuity,
        MissingSleepNight,
        Logical
    };

    struct Error {
        int line_number;
        std::string message;
        ErrorType type;
        bool operator<(const Error& other) const {
            if (line_number != other.line_number) return line_number < other.line_number;
            if (type != other.type) return type < other.type;
            return message < other.message;
        }
    };

    // 修改：构造函数接收新的配置文件
    FormatValidator(const std::string& config_filename, const std::string& header_config_filename);

    bool validateFile(const std::string& file_path, std::set<Error>& errors);

private:
    // ... Config 结构体保持不变 ...
    struct Config {
        std::map<std::string, std::unordered_set<std::string>> parent_categories;
        bool loaded = false;
    };

    struct DateBlock; // Forward declaration

    // --- Configuration and state ---
    std::string config_filepath_;
    std::string header_config_filepath_; // 新增
    Config config_;
    std::vector<std::string> header_order_; // 新增

    // --- Private helper methods ---
    void loadConfiguration();
    std::string trim(const std::string& str);
    bool parse_time_format(const std::string& time_str, int& hours, int& minutes);

    // --- Validation logic helpers ---
    void validate_date_line(const std::string& line, int line_num, DateBlock& block, std::set<Error>& errors);
    void validate_status_line(const std::string& line, int line_num, DateBlock& block, std::set<Error>& errors);
    void validate_sleep_line(const std::string& line, int line_num, DateBlock& block, std::set<Error>& errors); // 新增
    void validate_getup_line(const std::string& line, int line_num, DateBlock& block, std::set<Error>& errors);
    void validate_remark_line(const std::string& line, int line_num, DateBlock& block, std::set<Error>& errors);
    void validate_activity_line(const std::string& line, int line_num, DateBlock& block, std::set<Error>& errors);
    void finalize_block_status_validation(DateBlock& block, std::set<Error>& errors);
};

#endif // FORMAT_VALIDATOR_H