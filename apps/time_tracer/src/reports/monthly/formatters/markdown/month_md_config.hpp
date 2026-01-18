// reports/monthly/formatters/markdown/month_md_config.hpp
#ifndef REPORTS_MONTHLY_FORMATTERS_MARKDOWN_MONTH_MD_CONFIG_HPP_
#define REPORTS_MONTHLY_FORMATTERS_MARKDOWN_MONTH_MD_CONFIG_HPP_

#include "reports/monthly/common/month_base_config.hpp"
#include <string> 
#include <toml++/toml.h>

class MonthMdConfig : public MonthBaseConfig {
public:
    explicit MonthMdConfig(const toml::table& config);
    
    const std::string& get_project_breakdown_label() const;

private:
    std::string project_breakdown_label_;
};

#endif // REPORTS_MONTHLY_FORMATTERS_MARKDOWN_MONTH_MD_CONFIG_HPP_