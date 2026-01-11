// cli/CliParser.hpp
#ifndef CLI_PARSER_HPP
#define CLI_PARSER_HPP

#include <string>
#include <vector>
#include <optional>
#include "reports/shared/types/ReportFormat.hpp"
#include "converter/validator/common/ValidatorUtils.hpp"

class CliParser {
public:
    explicit CliParser(const std::vector<std::string>& args);

    std::string get_command() const;
    const std::vector<std::string>& get_filtered_args() const;
    std::string get_raw_arg(size_t index) const;

    std::optional<std::string> get_output_path() const;
    
    // 现在返回格式列表
    std::vector<ReportFormat> get_report_formats() const;

    DateCheckMode get_date_check_mode() const;
    // 检查是否包含保存参数
    bool should_save_processed() const;

private:
    std::vector<std::string> raw_args_;
    std::string command_;
    std::vector<std::string> filtered_args_;

    void parse();
    static std::vector<std::string> filter_global_options(const std::vector<std::string>& original_args);
};

#endif // CLI_PARSER_HPP