// reports/period/formatters/markdown/PeriodMdConfig.cpp
#include "PeriodMdConfig.hpp"

// [修改] 接收 JSON
PeriodMdConfig::PeriodMdConfig(const nlohmann::json& config)
    : PeriodBaseConfig(config) 
{
    project_breakdown_label_ = config_json_.value("project_breakdown_label", "Project Breakdown");
}

const std::string& PeriodMdConfig::get_project_breakdown_label() const {
    return project_breakdown_label_;
}