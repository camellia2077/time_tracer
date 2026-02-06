// config/validator/converter/rules/duration_rule.hpp
#ifndef CONFIG_VALIDATOR_CONVERTER_RULES_DURATION_RULE_H_
#define CONFIG_VALIDATOR_CONVERTER_RULES_DURATION_RULE_H_

#include <toml++/toml.h>

class DurationRule {
 public:
  static auto Validate(const toml::table& duration_tbl) -> bool;
};

#endif  // CONFIG_VALIDATOR_CONVERTER_RULES_DURATION_RULE_H_