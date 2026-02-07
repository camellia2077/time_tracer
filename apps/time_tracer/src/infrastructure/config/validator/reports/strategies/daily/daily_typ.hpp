// infrastructure/config/validator/reports/strategies/daily/daily_typ.hpp
#ifndef CONFIG_VALIDATOR_REPORTS_STRATEGIES_DAILY_DAILY_TYP_H_
#define CONFIG_VALIDATOR_REPORTS_STRATEGIES_DAILY_DAILY_TYP_H_

#include "infrastructure/config/validator/reports/strategies/base_strategy.hpp"

class DailyTyp : public BaseStrategy {
 protected:
  [[nodiscard]] auto ValidateSpecificKeys(const toml::table& query_config,
                                          const std::string& file_name) const
      -> bool override;
};

#endif  // CONFIG_VALIDATOR_REPORTS_STRATEGIES_DAILY_DAILY_TYP_H_