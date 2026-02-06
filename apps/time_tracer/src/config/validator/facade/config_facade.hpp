// config/validator/facade/config_facade.hpp
#ifndef CONFIG_VALIDATOR_FACADE_CONFIG_FACADE_H_
#define CONFIG_VALIDATOR_FACADE_CONFIG_FACADE_H_

#include <toml++/toml.h>

#include <filesystem>
#include <string>
#include <vector>

class ConfigFacade {
 public:
  static auto ValidateConverterConfigs(const toml::table& main_tbl,
                                       const toml::table& mappings_tbl,
                                       const toml::table& duration_rules_tbl)
      -> bool;

  // [修改] Reports 验证现在也是 TOML
  static auto ValidateQueryConfigs(
      const std::vector<std::pair<std::string, toml::table>>& query_configs)
      -> bool;

  static auto ValidatePlugins(const std::filesystem::path& plugins_path)
      -> bool;
};

#endif  // CONFIG_VALIDATOR_FACADE_CONFIG_FACADE_H_