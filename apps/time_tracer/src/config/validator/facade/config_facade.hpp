// config/validator/facade/config_facade.hpp
#ifndef CONFIG_VALIDATOR_FACADE_CONFIG_FACADE_H_
#define CONFIG_VALIDATOR_FACADE_CONFIG_FACADE_H_

#include <toml++/toml.h>

#include <filesystem>
#include <string>
#include <vector>

class ConfigFacade {
 public:
  static bool validate_converter_configs(const toml::table& main_tbl,
                                         const toml::table& mappings_tbl,
                                         const toml::table& duration_rules_tbl);

  // [修改] Reports 验证现在也是 TOML
  static bool validate_query_configs(
      const std::vector<std::pair<std::string, toml::table>>& query_configs);

  static bool validate_plugins(const std::filesystem::path& plugins_path);
};

#endif  // CONFIG_VALIDATOR_FACADE_CONFIG_FACADE_H_