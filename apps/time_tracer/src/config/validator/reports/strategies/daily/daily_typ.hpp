// config/validator/reports/strategies/daily/daily_typ.hpp
#ifndef CONFIG_VALIDATOR_REPORTS_STRATEGIES_DAILY_DAILY_TYP_HPP_
#define CONFIG_VALIDATOR_REPORTS_STRATEGIES_DAILY_DAILY_TYP_HPP_

#include "config/validator/reports/strategies/base_strategy.hpp"

class DailyTyp : public BaseStrategy {
protected:
    bool validate_specific_keys(const toml::table& query_config, const std::string& file_name) const override;
};

#endif // CONFIG_VALIDATOR_REPORTS_STRATEGIES_DAILY_DAILY_TYP_HPP_