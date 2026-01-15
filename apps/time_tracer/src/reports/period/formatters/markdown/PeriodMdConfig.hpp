// reports/period/formatters/markdown/PeriodMdConfig.hpp
#ifndef PERIOD_MD_CONFIG_HPP
#define PERIOD_MD_CONFIG_HPP

#include "reports/period/common/PeriodBaseConfig.hpp"
#include <string>
#include <toml++/toml.h> // [修改]

class PeriodMdConfig : public PeriodBaseConfig {
public:
    // [修改] 接收 toml::table
    explicit PeriodMdConfig(const toml::table& config);

    const std::string& get_project_breakdown_label() const;

private:
    std::string project_breakdown_label_;
};

#endif // PERIOD_MD_CONFIG_HPP