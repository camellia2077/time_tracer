// reports/period/formatters/md/PeriodMdConfig.hpp
#ifndef PERIOD_MD_CONFIG_HPP
#define PERIOD_MD_CONFIG_HPP

#include "reports/period/formatters/base/PeriodBaseConfig.hpp"

class PeriodMdConfig : public PeriodBaseConfig {
public:
    explicit PeriodMdConfig(const std::string& config_path);
};

#endif // PERIOD_MD_CONFIG_HPP