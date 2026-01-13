// reports/daily/formatters/latex/DayTexConfig.cpp
#include "DayTexConfig.hpp"

DayTexConfig::DayTexConfig(const std::filesystem::path& config_path)
    : DayBaseConfig(config_path), // 加载通用配置
      style_(config_json_)        // 加载样式配置 [核心修改]
{
    // 加载 Day 特有的
    report_title_ = config_json_.at("report_title").get<std::string>();
    keyword_colors_ = config_json_.at("keyword_colors").get<std::map<std::string, std::string>>();
}

const std::string& DayTexConfig::get_report_title() const { return report_title_; }
const std::map<std::string, std::string>& DayTexConfig::get_keyword_colors() const { return keyword_colors_; }