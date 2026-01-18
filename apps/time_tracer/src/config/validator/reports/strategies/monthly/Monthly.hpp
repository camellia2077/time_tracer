// config/validator/reports/strategies/monthly/monthly.hpp
#ifndef CONFIG_VALIDATOR_REPORTS_STRATEGIES_MONTHLY_MONTHLY_HPP_
#define CONFIG_VALIDATOR_REPORTS_STRATEGIES_MONTHLY_MONTHLY_HPP_

#include "config/validator/reports/strategies/base_strategy.hpp"

class Monthly : public BaseStrategy {
protected:
    bool validate_specific_keys(const toml::table& query_config, const std::string& file_name) const override;
};

#endif // CONFIG_VALIDATOR_REPORTS_STRATEGIES_MONTHLY_MONTHLY_HPP_