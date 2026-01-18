// reports/monthly/formatters/markdown/month_md_config.cpp
#include "month_md_config.hpp"

MonthMdConfig::MonthMdConfig(const toml::table& config)
    : MonthBaseConfig(config) 
{
    project_breakdown_label_ = config_table_["project_breakdown_label"].value_or<std::string>("Project Breakdown");
}

const std::string& MonthMdConfig::get_project_breakdown_label() const {
    return project_breakdown_label_;
}