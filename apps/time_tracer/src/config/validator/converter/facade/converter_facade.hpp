// config/validator/converter/facade/converter_facade.hpp
#ifndef CONFIG_VALIDATOR_CONVERTER_FACADE_CONVERTER_FACADE_H_
#define CONFIG_VALIDATOR_CONVERTER_FACADE_CONVERTER_FACADE_H_

#include <toml++/toml.h>

class ConverterFacade {
 public:
  // NOLINTBEGIN(bugprone-easily-swappable-parameters)
  static auto Validate(const toml::table& main_config,
                       const toml::table& mappings_config,
                       const toml::table& duration_rules_config) -> bool;

  // NOLINTEND(bugprone-easily-swappable-parameters)
};

#endif  // CONFIG_VALIDATOR_CONVERTER_FACADE_CONVERTER_FACADE_H_
