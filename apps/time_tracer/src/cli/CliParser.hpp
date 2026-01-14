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
    
    std::vector<ReportFormat> get_report_formats() const;

    // [修改] 返回 optional 以区分"未指定"和"指定为None"
    std::optional<DateCheckMode> get_date_check_mode() const;
    
    bool should_save_processed() const;
    bool should_skip_save() const;

private:
    std::vector<std::string> raw_args_;
    std::string command_;
    std::vector<std::string> filtered_args_;

    void parse();
    static std::vector<std::string> filter_global_options(const std::vector<std::string>& original_args);
};

#endif // CLI_PARSER_HPP