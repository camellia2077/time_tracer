// reports/period/formatters/markdown/PeriodMdConfig.cpp
#include "PeriodMdConfig.hpp"

// [修改] 接收 toml::table
PeriodMdConfig::PeriodMdConfig(const toml::table& config)
    : PeriodBaseConfig(config) 
{
    project_breakdown_label_ = config_table_["project_breakdown_label"].value_or<std::string>("Project Breakdown");
}

const std::string& PeriodMdConfig::get_project_breakdown_label() const {
    return project_breakdown_label_;
}