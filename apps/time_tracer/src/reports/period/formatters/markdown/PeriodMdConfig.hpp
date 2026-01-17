// reports/period/formatters/markdown/PeriodMdConfig.hpp
#ifndef REPORTS_PERIOD_FORMATTERS_MARKDOWN_PERIOD_MD_CONFIG_HPP_
#define REPORTS_PERIOD_FORMATTERS_MARKDOWN_PERIOD_MD_CONFIG_HPP_

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

#endif // REPORTS_PERIOD_FORMATTERS_MARKDOWN_PERIOD_MD_CONFIG_HPP_