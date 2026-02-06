// config/validator/converter/rules/duration_rule.cpp
#include "config/validator/converter/rules/duration_rule.hpp"

#include <iostream>

auto DurationRule::Validate(const toml::table& duration_tbl) -> bool {

  if (!duration_tbl.contains("text_duration_mappings") ||
      !duration_tbl["text_duration_mappings"].is_table()) {
    std::cerr << "[Validator] Error: Duration rules config must contain a "
                 "'text_duration_mappings' table."
              << std::endl;
    return false;
  }

  if (!duration_tbl.contains("duration_mappings") ||
      !duration_tbl["duration_mappings"].is_table()) {
    std::cerr << "[Validator] Error: Duration rules config must contain a "
                 "'duration_mappings' table."
              << std::endl;
    return false;
  }

  const auto* mappings = duration_tbl["duration_mappings"].as_table();
  for (const auto& [key, node] : *mappings) {
    if (!node.is_array()) {
      std::cerr << "[Validator] Error: In duration rules, the value for key '"
                << key.str() << "' must be an array." << std::endl;
      return false;
    }

    const auto& rules_array = *node.as_array();
    for (const auto& rule_node : rules_array) {
      if (!rule_node.is_table()) {
        std::cerr << "[Validator] Error: In duration rules for '" << key.str()
                  << "', each rule must be an inline table." << std::endl;
        return false;
      }
      const auto& rule = *rule_node.as_table();

      if (!rule.contains("less_than_minutes") || !rule.contains("value")) {
        std::cerr << "[Validator] Error: Rules must have 'less_than_minutes' "
                     "and 'value'."
                  << std::endl;
        return false;
      }
      if (!rule["less_than_minutes"].is_integer()) {
        std::cerr
            << "[Validator] Error: 'less_than_minutes' must be an integer."
            << std::endl;
        return false;
      }
      if (!rule["value"].is_string()) {
        std::cerr << "[Validator] Error: 'value' must be a string."
                  << std::endl;
        return false;
      }
    }
  }
  return true;
}