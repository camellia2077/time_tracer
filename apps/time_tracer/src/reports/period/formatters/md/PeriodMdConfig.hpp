// reports/period/formatters/md/PeriodMdConfig.hpp
#ifndef PERIOD_MD_CONFIG_HPP
#define PERIOD_MD_CONFIG_HPP

#include "reports/period/formatters/base/PeriodBaseConfig.hpp"
#include <string> // [新增]

class PeriodMdConfig : public PeriodBaseConfig {
public:
    explicit PeriodMdConfig(const std::string& config_path);

    // [新增] Getter 方法
    const std::string& get_project_breakdown_label() const;

private:
    // [新增] 成员变量
    std::string project_breakdown_label_;
};

#endif // PERIOD_MD_CONFIG_HPP