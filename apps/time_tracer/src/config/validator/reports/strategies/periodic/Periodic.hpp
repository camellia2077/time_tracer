// config/validator/reports/strategies/periodic/periodic.hpp
#ifndef CONFIG_VALIDATOR_REPORTS_STRATEGIES_PERIODIC_PERIODIC_HPP_
#define CONFIG_VALIDATOR_REPORTS_STRATEGIES_PERIODIC_PERIODIC_HPP_

#include "config/validator/reports/strategies/base_strategy.hpp"

class Periodic : public BaseStrategy {
protected:
    bool validate_specific_keys(const toml::table& query_config, const std::string& file_name) const override;
};

#endif // CONFIG_VALIDATOR_REPORTS_STRATEGIES_PERIODIC_PERIODIC_HPP_