// infrastructure/config/validator/converter/rules/converter_rules.hpp
#ifndef INFRASTRUCTURE_CONFIG_VALIDATOR_CONVERTER_RULES_CONVERTER_RULES_H_
#define INFRASTRUCTURE_CONFIG_VALIDATOR_CONVERTER_RULES_CONVERTER_RULES_H_

#include <toml++/toml.h>

#include <string>

class MainRule {
 public:
  // NOLINTBEGIN(bugprone-easily-swappable-parameters)
  static auto Validate(const toml::table& main_tbl,
                       std::string& out_mappings_path,
                       std::string& out_duration_rules_path) -> bool;
  // NOLINTEND(bugprone-easily-swappable-parameters)
};

class DurationRule {
 public:
  static auto Validate(const toml::table& duration_tbl) -> bool;
};

class MappingRule {
 public:
  static auto Validate(const toml::table& mappings_tbl) -> bool;
};

#endif  // INFRASTRUCTURE_CONFIG_VALIDATOR_CONVERTER_RULES_CONVERTER_RULES_H_
