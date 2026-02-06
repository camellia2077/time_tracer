// config/validator/reports/strategies/daily/daily_md.hpp
#ifndef CONFIG_VALIDATOR_REPORTS_STRATEGIES_DAILY_DAILY_MD_H_
#define CONFIG_VALIDATOR_REPORTS_STRATEGIES_DAILY_DAILY_MD_H_

#include "config/validator/reports/strategies/base_strategy.hpp"

class DailyMd : public BaseStrategy {
 protected:
  [[nodiscard]] auto ValidateSpecificKeys(const toml::table& query_config,
                                          const std::string& file_name) const
      -> bool override;
};

#endif  // CONFIG_VALIDATOR_REPORTS_STRATEGIES_DAILY_DAILY_MD_H_