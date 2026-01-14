// reports/monthly/formatters/markdown/MonthMdConfig.cpp
#include "MonthMdConfig.hpp"

// [修改] 接收 JSON
MonthMdConfig::MonthMdConfig(const nlohmann::json& config)
    : MonthBaseConfig(config) 
{
    project_breakdown_label_ = config_json_.value("project_breakdown_label", "Project Breakdown");
}

const std::string& MonthMdConfig::get_project_breakdown_label() const {
    return project_breakdown_label_;
}