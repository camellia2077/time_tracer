// config/validator/reports/strategies/yearly/yearly.hpp
#ifndef CONFIG_VALIDATOR_REPORTS_STRATEGIES_YEARLY_YEARLY_H_
#define CONFIG_VALIDATOR_REPORTS_STRATEGIES_YEARLY_YEARLY_H_

#include "config/validator/reports/strategies/base_strategy.hpp"

class Yearly : public BaseStrategy {
 protected:
  [[nodiscard]] auto ValidateSpecificKeys(const toml::table& query_config,
                                          const std::string& file_name) const
      -> bool override;
};

#endif  // CONFIG_VALIDATOR_REPORTS_STRATEGIES_YEARLY_YEARLY_H_
