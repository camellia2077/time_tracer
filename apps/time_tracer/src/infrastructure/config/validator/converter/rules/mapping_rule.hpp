// infrastructure/config/validator/converter/rules/mapping_rule.hpp
#ifndef CONFIG_VALIDATOR_CONVERTER_RULES_MAPPING_RULE_H_
#define CONFIG_VALIDATOR_CONVERTER_RULES_MAPPING_RULE_H_

#include <toml++/toml.h>

class MappingRule {
 public:
  static auto Validate(const toml::table& mappings_tbl) -> bool;
};

#endif  // CONFIG_VALIDATOR_CONVERTER_RULES_MAPPING_RULE_H_