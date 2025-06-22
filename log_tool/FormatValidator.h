// FormatValidator.h

#ifndef FORMAT_VALIDATOR_H
#define FORMAT_VALIDATOR_H

#include <string>
#include <vector>
#include <set>
#include <unordered_set>
#include <map>
#include <utility> // For std::pair
#include <nlohmann/json.hpp>

class FormatValidator {
public:
    enum class ErrorType {
        FileAccess,
        Structural,
        LineFormat,
        TimeDiscontinuity,
        MissingSleepNight,
        Logical,
        DateContinuity,
        IncorrectDayCountForMonth
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

    // --- 修改：更新构造函数声明 ---
    FormatValidator(const std::string& config_filename, const std::string& header_config_filename, bool enable_day_count_check = false);
    bool validateFile(const std::string& file_path, std::set<Error>& errors);

private:
    // --- START CORRECTION ---
    // The full definition of DateBlock is moved here from the .cpp file.
    // The forward declaration is no longer needed.
    struct DateBlock {
        int start_line_number = -1;
        int date_line_num = -1;
        std::string date_str;
        bool header_completely_valid = true;
        bool sleep_value_from_file = false;
        // Stores pairs of <activity_name, line_number>
        std::vector<std::pair<std::string, int>> activity_lines_content;

        // Reset method to clear the state for the next block.
        void reset() {
            start_line_number = -1;
            date_line_num = -1;
            date_str.clear();
            header_completely_valid = true;
            sleep_value_from_file = false;
            activity_lines_content.clear();
        }
    };
    // --- END CORRECTION ---

    struct Config {
        std::map<std::string, std::unordered_set<std::string>> parent_categories;
        bool loaded = false;
    };

    // --- Configuration and state ---
    std::string config_filepath_;
    std::string header_config_filepath_;
    Config config_;
    std::vector<std::string> header_order_;
    std::string previous_date_str_;
    bool check_day_count_enabled_; // --- 新增：用于存储天数检查的状态 ---

    // --- Private helper methods ---
    void loadConfiguration();
    std::string trim(const std::string& str);
    bool parse_time_format(const std::string& time_str, int& hours, int& minutes);
    
    // Date helper methods
    bool is_leap(int year);
    int days_in_month(int year, int month);
    std::string increment_date(const std::string& date_str);

    // --- Validation logic helpers ---
    void validate_date_line(const std::string& line, int line_num, DateBlock& block, std::set<Error>& errors);
    void validate_status_line(const std::string& line, int line_num, DateBlock& block, std::set<Error>& errors);
    void validate_sleep_line(const std::string& line, int line_num, DateBlock& block, std::set<Error>& errors);
    void validate_getup_line(const std::string& line, int line_num, DateBlock& block, std::set<Error>& errors);
    void validate_remark_line(const std::string& line, int line_num, DateBlock& block, std::set<Error>& errors);
    void validate_activity_line(const std::string& line, int line_num, DateBlock& block, std::set<Error>& errors);
    void finalize_block_status_validation(DateBlock& block, std::set<Error>& errors);
    
    // Final validation check for all dates in the file
    void validate_all_days_for_month(const std::map<std::string, std::set<int>>& month_day_map, std::set<Error>& errors);
};

#endif // FORMAT_VALIDATOR_H