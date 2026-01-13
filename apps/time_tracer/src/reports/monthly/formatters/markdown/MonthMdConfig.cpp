// reports/monthly/formatters/markdown/MonthMdConfig.cpp
#include "MonthMdConfig.hpp"

MonthMdConfig::MonthMdConfig(const std::filesystem::path& config_path)
    : MonthBaseConfig(config_path) 
{
    project_breakdown_label_ = config_json_.value("project_breakdown_label", "Project Breakdown");
}

const std::string& MonthMdConfig::get_project_breakdown_label() const {
    return project_breakdown_label_;
}