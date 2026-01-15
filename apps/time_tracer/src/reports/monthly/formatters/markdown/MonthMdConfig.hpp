// reports/monthly/formatters/markdown/MonthMdConfig.hpp
#ifndef MONTH_MD_CONFIG_HPP
#define MONTH_MD_CONFIG_HPP

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

#endif // MONTH_MD_CONFIG_HPP