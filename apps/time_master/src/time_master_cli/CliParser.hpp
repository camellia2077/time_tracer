// time_master_cli/CliParser.hpp
#ifndef CLI_PARSER_HPP
#define CLI_PARSER_HPP

#include <string>
#include <vector>
#include <optional>
#include "queries/shared/types/ReportFormat.hpp"

class CliParser {
public:
    explicit CliParser(const std::vector<std::string>& args);

    // 获取主命令
    std::string get_command() const;

    // 获取过滤掉全局选项后的参数列表
    const std::vector<std::string>& get_filtered_args() const;

    // 获取特定位置的原始参数
    std::string get_raw_arg(size_t index) const;

    // 解析输出路径选项
    std::optional<std::string> get_output_path() const;

    // 解析报告格式选项
    ReportFormat get_report_format() const;

private:
    std::vector<std::string> raw_args_;
    std::string command_;
    std::vector<std::string> filtered_args_;

    void parse();
    static std::vector<std::string> filter_global_options(const std::vector<std::string>& original_args);
};

#endif // CLI_PARSER_HPP