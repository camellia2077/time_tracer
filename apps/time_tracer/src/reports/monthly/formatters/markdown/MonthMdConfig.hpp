// reports/monthly/formatters/markdown/MonthMdConfig.hpp
#ifndef MONTH_MD_CONFIG_HPP
#define MONTH_MD_CONFIG_HPP

#include "reports/monthly/formatters/base/MonthBaseConfig.hpp"
#include <string> 
#include <filesystem>

class MonthMdConfig : public MonthBaseConfig {
public:
    explicit MonthMdConfig(const std::filesystem::path& config_path);
    
    const std::string& get_project_breakdown_label() const;

private:
    std::string project_breakdown_label_;
};

#endif // MONTH_MD_CONFIG_HPP