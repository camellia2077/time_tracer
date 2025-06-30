// FormatValidator.h (已重构)

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
        // 新增: 源文件特定的错误类型
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

    // 构造函数保持不变
    FormatValidator(const std::string& config_filename, const std::string& header_config_filename, bool enable_day_count_check = false);

    // #### 核心修改：定义两个独立的验证函数 ####
    /**
     * @brief 验证源文件格式 (例如 2023_01.txt)
     */
    bool validateSourceFile(const std::string& file_path, std::set<Error>& errors);

    /**
     * @brief 验证输出文件格式 (例如 final_... or processed_...)
     */
    bool validateOutputFile(const std::string& file_path, std::set<Error>& errors);

private:
    // 输出文件验证所需的数据结构
    struct DateBlock {
        int start_line_number = -1;
        int date_line_num = -1;
        std::string date_str;
        bool header_completely_valid = true;
        bool sleep_value_from_file = false;
        std::vector<std::pair<std::string, int>> activity_lines_content;
        void reset();
    };
    
    // 配置与状态
    struct Config {
        std::map<std::string, std::unordered_set<std::string>> parent_categories;
        bool loaded = false;
    };
    
    std::string config_filepath_;
    std::string header_config_filepath_;
    std::string remark_prefix_from_config_; // 新增: 存储从配置中读取的备注前缀
    Config config_;
    std::vector<std::string> header_order_;
    std::vector<std::pair<std::string, int>> collected_dates_;
    bool check_day_count_enabled_;

    // 私有辅助函数
    void loadConfiguration();
    std::string trim(const std::string& str);

    // --- 源文件验证的辅助函数 ---
    bool isSourceDateLine(const std::string& line);
    bool isSourceRemarkLine(const std::string& line);
    bool parseSourceEventLine(const std::string& line, std::string& outTimeStr, std::string& outDescription);

    // --- 输出文件验证的辅助函数 ---
    bool parse_time_format(const std::string& time_str, int& hours, int& minutes);
    bool is_leap(int year);
    int days_in_month(int year, int month);
    std::string increment_date(const std::string& date_str);
    void validate_date_line(const std::string& line, int line_num, DateBlock& block, std::set<Error>& errors);
    void validate_status_line(const std::string& line, int line_num, DateBlock& block, std::set<Error>& errors);
    void validate_sleep_line(const std::string& line, int line_num, DateBlock& block, std::set<Error>& errors);
    void validate_getup_line(const std::string& line, int line_num, DateBlock& block, std::set<Error>& errors);
    void validate_remark_line(const std::string& line, int line_num, DateBlock& block, std::set<Error>& errors);
    void validate_activity_line(const std::string& line, int line_num, DateBlock& block, std::set<Error>& errors);
    void finalize_block_status_validation(DateBlock& block, std::set<Error>& errors);
    void validate_date_continuity(std::set<Error>& errors);
    void validate_month_start(const std::map<std::string, std::set<int>>& month_day_map, std::set<Error>& errors);
    void validate_all_days_for_month(const std::map<std::string, std::set<int>>& month_day_map, std::set<Error>& errors);
};

#endif // FORMAT_VALIDATOR_H