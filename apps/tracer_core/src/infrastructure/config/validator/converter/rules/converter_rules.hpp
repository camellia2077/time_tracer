// infrastructure/config/validator/converter/rules/converter_rules.hpp
#ifndef INFRASTRUCTURE_CONFIG_VALIDATOR_CONVERTER_RULES_CONVERTER_RULES_H_
#define INFRASTRUCTURE_CONFIG_VALIDATOR_CONVERTER_RULES_CONVERTER_RULES_H_

#include <toml++/toml.h>

#include <string>

struct MainConfigPaths {
  std::string duration_rules_config_path;
  std::string alias_mapping_path;
};

class MainRule {
 public:
  static auto Validate(const toml::table& main_tbl, MainConfigPaths& out_paths)
      -> bool;
};

class DurationRule {
 public:
  static auto Validate(const toml::table& duration_tbl) -> bool;
};

class MappingRule {
 public:
  static auto Validate(const toml::table& mappings_tbl) -> bool;
};

class V2Rule {
 public:
  static auto ValidateAliasMapping(const toml::table& alias_tbl) -> bool;
};

#endif  // INFRASTRUCTURE_CONFIG_VALIDATOR_CONVERTER_RULES_CONVERTER_RULES_H_
