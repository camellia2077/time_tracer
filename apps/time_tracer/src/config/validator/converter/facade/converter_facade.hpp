// config/validator/converter/facade/converter_facade.hpp
#ifndef CONFIG_VALIDATOR_CONVERTER_FACADE_CONVERTER_FACADE_H_
#define CONFIG_VALIDATOR_CONVERTER_FACADE_CONVERTER_FACADE_H_

#include <toml++/toml.h>

class ConverterFacade {
 public:
  static bool validate(const toml::table& main_tbl,
                       const toml::table& mappings_tbl,
                       const toml::table& duration_rules_tbl);
};

#endif  // CONFIG_VALIDATOR_CONVERTER_FACADE_CONVERTER_FACADE_H_