// FormatValidator.h (修改后)
#ifndef FORMAT_VALIDATOR_H
#define FORMAT_VALIDATOR_H

#include <string>
#include <vector>
#include <map>
#include <set>
#include <unordered_set>
#include "json.hpp" // Assumes nlohmann/json.hpp is available

class FormatValidator {
public:
    // 定义错误类型
    enum class ErrorType {
        FileAccess,
        Structural,
        LineFormat,
        TimeDiscontinuity,
        MissingSleepNight,
        Logical
    };

    // Structure to represent a single validation error
    struct Error {
        int line_number;
        std::string message;
        ErrorType type;

        // 为方便在 std::set 中排序，重载 < 运算符
        bool operator<(const Error& other) const {
            if (line_number != other.line_number) return line_number < other.line_number;
            if (type != other.type) return type < other.type;
            return message < other.message;
        }
    };

    // Constructor: Initializes the validator with its configuration file.
    FormatValidator(const std::string& config_filename);

    // Validates the specified file.
    // Returns true if the file is valid, false otherwise.
    // The 'errors' parameter will be populated with any issues found.
    bool validateFile(const std::string& file_path, std::set<Error>& errors);

private:
    // Configuration structure
    struct Config {
        std::map<std::string, std::unordered_set<std::string>> parent_categories;
        bool loaded = false;
    };

    // Structure for parsing a date block
    struct DateBlock; // Forward declaration, defined in .cpp

    // Configuration and state
    std::string config_filepath_;
    Config config_;

    // Private helper methods
    void loadConfiguration();
    std::string trim(const std::string& str);
    bool parse_time_format(const std::string& time_str, int& hours, int& minutes);

    // Validation logic helpers (specific to parsing states)
    void validate_date_line(const std::string& line, int line_num, DateBlock& block, std::set<Error>& errors);
    void validate_status_line(const std::string& line, int line_num, DateBlock& block, std::set<Error>& errors);
    void validate_getup_line(const std::string& line, int line_num, DateBlock& block, std::set<Error>& errors);
    void validate_remark_line(const std::string& line, int line_num, DateBlock& block, std::set<Error>& errors);
    void validate_activity_line(const std::string& line, int line_num, DateBlock& block, std::set<Error>& errors);
    void finalize_block_status_validation(DateBlock& block, std::set<Error>& errors);
};

#endif // FORMAT_VALIDATOR_H