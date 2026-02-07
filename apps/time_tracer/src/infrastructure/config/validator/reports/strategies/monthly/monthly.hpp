// infrastructure/config/validator/reports/strategies/monthly/monthly.hpp
#ifndef CONFIG_VALIDATOR_REPORTS_STRATEGIES_MONTHLY_MONTHLY_H_
#define CONFIG_VALIDATOR_REPORTS_STRATEGIES_MONTHLY_MONTHLY_H_

#include "infrastructure/config/validator/reports/strategies/base_strategy.hpp"

class Monthly : public BaseStrategy {
 protected:
  [[nodiscard]] auto ValidateSpecificKeys(const toml::table& query_config,
                                          const std::string& file_name) const
      -> bool override;
};

#endif  // CONFIG_VALIDATOR_REPORTS_STRATEGIES_MONTHLY_MONTHLY_H_