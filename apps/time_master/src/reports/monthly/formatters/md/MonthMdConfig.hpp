// reports/monthly/formatters/md/MonthMdConfig.hpp
#ifndef MONTH_MD_CONFIG_HPP
#define MONTH_MD_CONFIG_HPP

#include "reports/monthly/formatters/base/MonthBaseConfig.hpp"

class MonthMdConfig : public MonthBaseConfig {
public:
    explicit MonthMdConfig(const std::string& config_path);
};

#endif // MONTH_MD_CONFIG_HPP