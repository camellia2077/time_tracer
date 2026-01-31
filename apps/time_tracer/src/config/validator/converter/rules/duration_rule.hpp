// config/validator/converter/rules/duration_rule.hpp
#ifndef CONFIG_VALIDATOR_CONVERTER_RULES_DURATION_RULE_H_
#define CONFIG_VALIDATOR_CONVERTER_RULES_DURATION_RULE_H_

#include <toml++/toml.h>

class DurationRule {
 public:
  static bool validate(const toml::table& duration_tbl);
};

#endif  // CONFIG_VALIDATOR_CONVERTER_RULES_DURATION_RULE_H_