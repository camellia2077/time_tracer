// infrastructure/config/config.hpp
#ifndef INFRASTRUCTURE_CONFIG_CONFIG_H_
#define INFRASTRUCTURE_CONFIG_CONFIG_H_

#include <optional>
#include <string>
#include <toml++/toml.hpp>

#include "common/config_types.hpp"

class ConfigLoader {
 public:
  static std::optional<TomlConfigData> load_from_content(
      const std::string& settings_content, const std::string& mapping_content);

 private:
  static bool _parse_mapping_keys(const std::string& content,
                                  TomlConfigData& config_data);
  static bool _parse_settings(const std::string& content,
                              TomlConfigData& config_data);

  static void _load_daily_remarks(const toml::table& data,
                                  TomlConfigData& config);
  static void _load_activity_remarks(const toml::table& data,
                                     TomlConfigData& config);
  static void _load_wake_keywords(const toml::table& data,
                                  TomlConfigData& config);
  static void _load_nosleep_settings(const toml::table& data,
                                     TomlConfigData& config);
};

#endif  // INFRASTRUCTURE_CONFIG_CONFIG_H_
