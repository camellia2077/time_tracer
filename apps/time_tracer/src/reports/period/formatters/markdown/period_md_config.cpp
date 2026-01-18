// reports/period/formatters/markdown/period_md_config.cpp
#include "period_md_config.hpp"

namespace reporting {

PeriodMdConfig::PeriodMdConfig(const toml::table& config)
    : PeriodBaseConfig(config) 
{
    project_breakdown_label_ = config_table_["project_breakdown_label"].value_or<std::string>("Project Breakdown");
}

const std::string& PeriodMdConfig::get_project_breakdown_label() const {
    return project_breakdown_label_;
}

} // namespace reporting