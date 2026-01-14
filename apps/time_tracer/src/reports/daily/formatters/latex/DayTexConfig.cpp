// reports/daily/formatters/latex/DayTexConfig.cpp
#include "DayTexConfig.hpp"

// [修改] 接收 JSON，并用来初始化基类和成员
DayTexConfig::DayTexConfig(const nlohmann::json& config)
    : DayBaseConfig(config), // 传递给基类
      style_(config)         // 传递给 TexStyleConfig (它本身也接受 json)
{
    report_title_ = config_json_.at("report_title").get<std::string>();
    keyword_colors_ = config_json_.at("keyword_colors").get<std::map<std::string, std::string>>();
}

const std::string& DayTexConfig::get_report_title() const { return report_title_; }
const std::map<std::string, std::string>& DayTexConfig::get_keyword_colors() const { return keyword_colors_; }