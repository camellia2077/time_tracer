// reports/daily/formatters/typst/DayTypConfig.cpp
#include "DayTypConfig.hpp"

DayTypConfig::DayTypConfig(const std::filesystem::path& config_path)
    : DayBaseConfig(config_path),
      style_(config_json_) // [核心修改]
{
    statistic_font_size_ = config_json_.at("statistic_font_size").get<int>();
    statistic_title_font_size_ = config_json_.at("statistic_title_font_size").get<int>();
    keyword_colors_ = config_json_.at("keyword_colors").get<std::map<std::string, std::string>>();
}

int DayTypConfig::get_statistic_font_size() const { return statistic_font_size_; }
int DayTypConfig::get_statistic_title_font_size() const { return statistic_title_font_size_; }
const std::map<std::string, std::string>& DayTypConfig::get_keyword_colors() const { return keyword_colors_; }