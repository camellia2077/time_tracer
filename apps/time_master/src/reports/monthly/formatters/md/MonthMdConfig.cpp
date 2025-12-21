#include "MonthMdConfig.hpp"

MonthMdConfig::MonthMdConfig(const std::string& config_path)
    : MonthBaseConfig(config_path) 
{
    // [新增] 从 config_json_ (继承自 BaseConfig) 读取配置
    // 如果 JSON 中没有配这个字段，默认使用 "Project Breakdown"
    project_breakdown_label_ = config_json_.value("project_breakdown_label", "Project Breakdown");
}

// [新增] Getter 实现
const std::string& MonthMdConfig::get_project_breakdown_label() const {
    return project_breakdown_label_;
}