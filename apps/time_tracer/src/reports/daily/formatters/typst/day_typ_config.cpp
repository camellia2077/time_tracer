// reports/daily/formatters/typst/day_typ_config.cpp
#include "day_typ_config.hpp"

DayTypConfig::DayTypConfig(const toml::table& config)
    : DayBaseConfig(config),
      style_(config)
{
    statistic_font_size_ = config_table_["statistic_font_size"].value_or(kDefaultStatFontSize);
    statistic_title_font_size_ = config_table_["statistic_title_font_size"].value_or(kDefaultStatTitleFontSize);
    
    if (const toml::table* kw_tbl = config_table_["keyword_colors"].as_table()) {
        for (const auto& [key, val] : *kw_tbl) {
            if (auto color_val = val.value<std::string>()) {
                // 显式转换为 std::string 以匹配 map 键类型
                keyword_colors_[std::string(key.str())] = *color_val;
            }
        }
    }
}

auto DayTypConfig::get_statistic_font_size() const -> int { return statistic_font_size_; }
auto DayTypConfig::get_statistic_title_font_size() const -> int { return statistic_title_font_size_; }
auto DayTypConfig::get_keyword_colors() const
    -> const std::map<std::string, std::string>& {
    return keyword_colors_;
}
