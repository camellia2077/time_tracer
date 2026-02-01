// config/validator/converter/facade/converter_facade.cpp
#include "converter_facade.hpp"

#include <iostream>

#include "config/validator/converter/rules/duration_rule.hpp"
#include "config/validator/converter/rules/main_rule.hpp"
#include "config/validator/converter/rules/mapping_rule.hpp"

// NOLINTBEGIN(bugprone-easily-swappable-parameters)
auto ConverterFacade::validate(const toml::table& main_config,
                               const toml::table& mappings_config,
                               const toml::table& duration_rules_config)
    -> bool {
  std::string mappings_path_str;
  std::string duration_rules_path_str;

  if (!MainRule::validate(main_config, mappings_path_str,
                          duration_rules_path_str)) {
    return false;
  }

  if (!MappingRule::validate(mappings_config)) {
    return false;
  }

  if (!DurationRule::validate(duration_rules_config)) {
    return false;
  }

  std::cout
      << "[Validator] All preprocessing configuration data (TOML) is valid."
      << std::endl;
  return true;
}
// NOLINTEND(bugprone-easily-swappable-parameters)
