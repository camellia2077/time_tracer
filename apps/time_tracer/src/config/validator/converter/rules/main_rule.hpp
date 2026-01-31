// config/validator/converter/rules/main_rule.hpp
#ifndef CONFIG_VALIDATOR_CONVERTER_RULES_MAIN_RULE_H_
#define CONFIG_VALIDATOR_CONVERTER_RULES_MAIN_RULE_H_

#include <toml++/toml.h>

#include <string>

class MainRule {
 public:
  static bool validate(const toml::table& main_tbl,
                       std::string& out_mappings_path,
                       std::string& out_duration_rules_path);
};

#endif  // CONFIG_VALIDATOR_CONVERTER_RULES_MAIN_RULE_H_