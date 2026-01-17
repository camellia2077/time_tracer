// reports/monthly/formatters/markdown/MonthMdConfig.hpp
#ifndef REPORTS_MONTHLY_FORMATTERS_MARKDOWN_MONTH_MD_CONFIG_HPP_
#define REPORTS_MONTHLY_FORMATTERS_MARKDOWN_MONTH_MD_CONFIG_HPP_

#include "reports/monthly/common/MonthBaseConfig.hpp"
#include <string> 
#include <toml++/toml.h> // [修改]

class MonthMdConfig : public MonthBaseConfig {
public:
    // [修改] 接收 toml::table
    explicit MonthMdConfig(const toml::table& config);
    
    const std::string& get_project_breakdown_label() const;

private:
    std::string project_breakdown_label_;
};

#endif // REPORTS_MONTHLY_FORMATTERS_MARKDOWN_MONTH_MD_CONFIG_HPP_