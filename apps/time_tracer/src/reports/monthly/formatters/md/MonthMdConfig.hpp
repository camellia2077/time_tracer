#ifndef MONTH_MD_CONFIG_HPP
#define MONTH_MD_CONFIG_HPP

#include "reports/monthly/formatters/base/MonthBaseConfig.hpp"
#include <string> // [新增]

class MonthMdConfig : public MonthBaseConfig {
public:
    explicit MonthMdConfig(const std::string& config_path);
    
    // [新增] Getter 方法
    const std::string& get_project_breakdown_label() const;

private:
    // [新增] 成员变量
    std::string project_breakdown_label_;
};

#endif // MONTH_MD_CONFIG_HPP