// infra/config/validator/converter/rules/converter_rules.hpp
#ifndef INFRASTRUCTURE_CONFIG_VALIDATOR_CONVERTER_RULES_CONVERTER_RULES_H_
#define INFRASTRUCTURE_CONFIG_VALIDATOR_CONVERTER_RULES_CONVERTER_RULES_H_

#include <toml++/toml.h>

#include <filesystem>
#include <string>

class MainRule {
 public:
  static auto Validate(const toml::table& main_tbl) -> bool;
};

class MappingRule {
 public:
  static auto Validate(const toml::table& mappings_tbl) -> bool;
};

class V2Rule {
 public:
  static auto ValidateAliasMapping(const std::filesystem::path& alias_directory)
      -> bool;
};

#endif  // INFRASTRUCTURE_CONFIG_VALIDATOR_CONVERTER_RULES_CONVERTER_RULES_H_
