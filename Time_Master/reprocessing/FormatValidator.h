#ifndef FORMAT_VALIDATOR_H
#define FORMAT_VALIDATOR_H

#include <string>
#include <vector>
#include <set>
#include <unordered_set>
#include <map>
#include <utility>
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
        IncorrectDayCountForMonth,
        Source_RemarkAfterEvent,
        Source_NoDateAtStart,
        Source_InvalidLineFormat
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

    FormatValidator(const std::string& config_filename, const std::string& header_config_filename, bool enable_day_count_check = false);
    bool validateSourceFile(const std::string& file_path, std::set<Error>& errors);
    bool validateOutputFile(const std::string& file_path, std::set<Error>& errors);
    static void printGroupedErrors(const std::string& filename, const std::set<Error>& errors, const std::string& error_log_path);

private:
    struct DateBlock {
        int start_line_number = -1;
        int date_line_num = -1;
        std::string date_str;
        bool header_completely_valid = true;
        bool sleep_value_from_file = false;
        
        // 【核心修改】这个 pair 现在代表 {活动名称, 行号}
        std::vector<std::pair<std::string, int>> activity_lines_content;
        
        // 【新增】单独存储上一个活动的结束时间，用于时间连续性检查
        std::string last_activity_end_time;
        
        void reset();
    };
    
    struct Config {
        std::map<std::string, std::unordered_set<std::string>> parent_categories;
        bool loaded = false;
    };
    
    std::string config_filepath_;
    std::string header_config_filepath_;
    std::string remark_prefix_from_config_;
    Config config_;
    std::vector<std::string> header_order_;
    std::vector<std::pair<std::string, int>> collected_dates_;
    bool check_day_count_enabled_;

    // 私有辅助函数
    void loadConfiguration();
    std::string trim(const std::string& str);
    bool is_valid_activity(const std::string& activity_name) const;

    // 源文件验证辅助
    bool isSourceDateLine(const std::string& line);
    bool isSourceRemarkLine(const std::string& line);
    bool parseSourceEventLine(const std::string& line, std::string& outTimeStr, std::string& outDescription);

    // 输出文件验证辅助
    bool parse_time_format(const std::string& time_str, int& hours, int& minutes);
    void validate_date_line(const std::string& line, int line_num, DateBlock& block, std::set<Error>& errors);
    void validate_status_line(const std::string& line, int line_num, DateBlock& block, std::set<Error>& errors);
    void validate_sleep_line(const std::string& line, int line_num, DateBlock& block, std::set<Error>& errors);
    void validate_getup_line(const std::string& line, int line_num, DateBlock& block, std::set<Error>& errors);
    void validate_remark_line(const std::string& line, int line_num, DateBlock& block, std::set<Error>& errors);
    void validate_activity_line(const std::string& line, int line_num, DateBlock& block, std::set<Error>& errors);
    void finalize_block_status_validation(DateBlock& block, std::set<Error>& errors);
    
    // 日期连续性检查辅助
    bool is_leap(int year);
    int days_in_month(int year, int month);
    std::string increment_date(const std::string& date_str);
    void validate_date_continuity(std::set<Error>& errors);
    void validate_month_start(const std::map<std::string, std::set<int>>& month_day_map, std::set<Error>& errors);
    void validate_all_days_for_month(const std::map<std::string, std::set<int>>& month_day_map, std::set<Error>& errors);

    static std::string getErrorTypeHeader(ErrorType type);
};

#endif // FORMAT_VALIDATOR_H