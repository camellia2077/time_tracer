#include "PeriodMdConfig.hpp"

PeriodMdConfig::PeriodMdConfig(const std::string& config_path)
    : PeriodBaseConfig(config_path) 
{
    // [新增] 从 config_json_ 读取配置
    project_breakdown_label_ = config_json_.value("project_breakdown_label", "Project Breakdown");
}

// [新增] Getter 实现
const std::string& PeriodMdConfig::get_project_breakdown_label() const {
    return project_breakdown_label_;
}