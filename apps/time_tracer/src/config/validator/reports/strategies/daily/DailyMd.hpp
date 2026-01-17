// config/validator/reports/strategies/daily/DailyMd.hpp
#ifndef CONFIG_VALIDATOR_REPORTS_STRATEGIES_DAILY_DAILY_MD_HPP_
#define CONFIG_VALIDATOR_REPORTS_STRATEGIES_DAILY_DAILY_MD_HPP_

#include "config/validator/reports/strategies/BaseStrategy.hpp"

class DailyMd : public BaseStrategy {
protected:
    bool validate_specific_keys(const toml::table& query_config, const std::string& file_name) const override;
};

#endif // CONFIG_VALIDATOR_REPORTS_STRATEGIES_DAILY_DAILY_MD_HPP_