// infrastructure/config/validator/reports/strategies/weekly/weekly.hpp
#ifndef CONFIG_VALIDATOR_REPORTS_STRATEGIES_WEEKLY_WEEKLY_H_
#define CONFIG_VALIDATOR_REPORTS_STRATEGIES_WEEKLY_WEEKLY_H_

#include "infrastructure/config/validator/reports/strategies/base_strategy.hpp"

class Weekly : public BaseStrategy {
 protected:
  [[nodiscard]] auto ValidateSpecificKeys(const toml::table& query_config,
                                          const std::string& file_name) const
      -> bool override;
};

#endif  // CONFIG_VALIDATOR_REPORTS_STRATEGIES_WEEKLY_WEEKLY_H_
