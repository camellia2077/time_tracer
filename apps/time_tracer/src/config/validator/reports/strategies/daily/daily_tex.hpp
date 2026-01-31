// config/validator/reports/strategies/daily/daily_tex.hpp
#ifndef CONFIG_VALIDATOR_REPORTS_STRATEGIES_DAILY_DAILY_TEX_H_
#define CONFIG_VALIDATOR_REPORTS_STRATEGIES_DAILY_DAILY_TEX_H_

#include "config/validator/reports/strategies/base_strategy.hpp"

class DailyTex : public BaseStrategy {
 protected:
  bool validate_specific_keys(const toml::table& query_config,
                              const std::string& file_name) const override;
};

#endif  // CONFIG_VALIDATOR_REPORTS_STRATEGIES_DAILY_DAILY_TEX_H_