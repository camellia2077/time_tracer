// reports/period/formatters/md/PeriodMdConfig.cpp
#include "PeriodMdConfig.hpp"


PeriodMdConfig::PeriodMdConfig(const std::filesystem::path& config_path)
    : PeriodBaseConfig(config_path) 
{
    project_breakdown_label_ = config_json_.value("project_breakdown_label", "Project Breakdown");
}

const std::string& PeriodMdConfig::get_project_breakdown_label() const {
    return project_breakdown_label_;
}