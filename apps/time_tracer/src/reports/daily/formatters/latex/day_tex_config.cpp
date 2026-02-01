// reports/daily/formatters/latex/day_tex_config.cpp
#include "day_tex_config.hpp"

DayTexConfig::DayTexConfig(const toml::table& config)
    : DayBaseConfig(config),
      style_(config)
{
    // config_table_ 在基类中是 protected，也可以直接用
    report_title_ = config_table_["report_title"].value_or("");
    
    if (const toml::table* kw_tbl = config_table_["keyword_colors"].as_table()) {
        for (const auto& [key, val] : *kw_tbl) {
            if (auto value_str = val.value<std::string>()) {
                // key.str() 返回的是 string_view，需要显式转换为 std::string
                // 才能作为 std::map<std::string, ...> 的键
                keyword_colors_[std::string(key.str())] = *value_str;
            }
        }
    }
}

auto DayTexConfig::get_report_title() const -> const std::string& {
    return report_title_;
}
auto DayTexConfig::get_keyword_colors() const
    -> const std::map<std::string, std::string>& {
    return keyword_colors_;
}
