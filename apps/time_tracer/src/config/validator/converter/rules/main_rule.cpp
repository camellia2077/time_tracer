// config/validator/converter/rules/main_rule.cpp
#include "config/validator/converter/rules/main_rule.hpp"

#include <iostream>
#include <set>

// NOLINTBEGIN(bugprone-easily-swappable-parameters)
auto MainRule::Validate(const toml::table& main_tbl,
                        std::string& out_mappings_path,
                        std::string& out_duration_rules_path) -> bool {

  const std::set<std::string> kRequiredKeys = {
      "mappings_config_path", "duration_rules_config_path",
      "top_parent_mapping",   "header_order",
      "remark_prefix",        "wake_keywords"};

  for (const auto& key : kRequiredKeys) {
    if (!main_tbl.contains(key)) {
      std::cerr << "[Validator] Error: Main config is missing required key: '"
                << key << "'" << std::endl;
      return false;
    }
  }

  // [修复] 类型检查 (TOML++)
  if (!main_tbl["mappings_config_path"].is_string() ||
      !main_tbl["duration_rules_config_path"].is_string()) {
    std::cerr << "[Validator] Error: 'mappings_config_path' and "
                 "'duration_rules_config_path' must be strings."
              << std::endl;
    return false;
  }
  if (!main_tbl["top_parent_mapping"].is_table()) {
    std::cerr << "[Validator] Error: 'top_parent_mapping' must be a Table."
              << std::endl;
    return false;
  }
  if (!main_tbl["header_order"].is_array() ||
      !main_tbl["wake_keywords"].is_array()) {
    std::cerr << "[Validator] Error: 'header_order' and 'wake_keywords' must "
                 "be arrays."
              << std::endl;
    return false;
  }
  if (!main_tbl["remark_prefix"].is_string()) {
    std::cerr << "[Validator] Error: 'remark_prefix' must be a string."
              << std::endl;
    return false;
  }

  // [修复] 提取值 (value<T>() 返回 std::optional)
  if (auto mappings_val =
          main_tbl["mappings_config_path"].value<std::string>()) {
    out_mappings_path = *mappings_val;
  }
  if (auto duration_val =
          main_tbl["duration_rules_config_path"].value<std::string>()) {
    out_duration_rules_path = *duration_val;
  }
  return true;
}
// NOLINTEND(bugprone-easily-swappable-parameters)
