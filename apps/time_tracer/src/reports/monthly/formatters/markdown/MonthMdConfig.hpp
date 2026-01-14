// reports/monthly/formatters/markdown/MonthMdConfig.hpp
#ifndef MONTH_MD_CONFIG_HPP
#define MONTH_MD_CONFIG_HPP

#include "reports/monthly/common/MonthBaseConfig.hpp"
#include <string> 
#include <nlohmann/json.hpp>

class MonthMdConfig : public MonthBaseConfig {
public:
    // [修改] 接收 JSON
    explicit MonthMdConfig(const nlohmann::json& config);
    
    const std::string& get_project_breakdown_label() const;

private:
    std::string project_breakdown_label_;
};

#endif // MONTH_MD_CONFIG_HPP