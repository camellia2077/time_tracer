// reports/daily/formatters/typst/day_typ_config.cpp
#include "day_typ_config.hpp"

DayTypConfig::DayTypConfig(const toml::table& config)
    : DayBaseConfig(config),
      style_(config)
{
    statistic_font_size_ = config_table_["statistic_font_size"].value_or(10);
    statistic_title_font_size_ = config_table_["statistic_title_font_size"].value_or(12);
    
    if (const toml::table* kw_tbl = config_table_["keyword_colors"].as_table()) {
        for (const auto& [key, val] : *kw_tbl) {
            if (auto s = val.value<std::string>()) {
                // 显式转换为 std::string 以匹配 map 键类型
                keyword_colors_[std::string(key.str())] = *s;
            }
        }
    }
}

int DayTypConfig::get_statistic_font_size() const { return statistic_font_size_; }
int DayTypConfig::get_statistic_title_font_size() const { return statistic_title_font_size_; }
const std::map<std::string, std::string>& DayTypConfig::get_keyword_colors() const { return keyword_colors_; }