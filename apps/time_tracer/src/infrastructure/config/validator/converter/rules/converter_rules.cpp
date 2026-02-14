// infrastructure/config/validator/converter/rules/converter_rules.cpp
#include "infrastructure/config/validator/converter/rules/converter_rules.hpp"

#include <set>
#include <string>

#include "domain/ports/diagnostics.hpp"

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
      time_tracer::domain::ports::EmitError(
          "[Validator] Error: Main config is missing required key: '" + key +
          "'");
      return false;
    }
  }

  if (!main_tbl["mappings_config_path"].is_string() ||
      !main_tbl["duration_rules_config_path"].is_string()) {
    time_tracer::domain::ports::EmitError(
        "[Validator] Error: 'mappings_config_path' and "
        "'duration_rules_config_path' must be strings.");
    return false;
  }
  if (!main_tbl["top_parent_mapping"].is_table()) {
    time_tracer::domain::ports::EmitError(
        "[Validator] Error: 'top_parent_mapping' must be a Table.");
    return false;
  }
  if (!main_tbl["header_order"].is_array() ||
      !main_tbl["wake_keywords"].is_array()) {
    time_tracer::domain::ports::EmitError(
        "[Validator] Error: 'header_order' and 'wake_keywords' must be "
        "arrays.");
    return false;
  }
  if (!main_tbl["remark_prefix"].is_string()) {
    time_tracer::domain::ports::EmitError(
        "[Validator] Error: 'remark_prefix' must be a string.");
    return false;
  }

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

auto DurationRule::Validate(const toml::table& duration_tbl) -> bool {
  if (!duration_tbl.contains("text_duration_mappings") ||
      !duration_tbl["text_duration_mappings"].is_table()) {
    time_tracer::domain::ports::EmitError(
        "[Validator] Error: Duration rules config must contain a "
        "'text_duration_mappings' table.");
    return false;
  }

  if (!duration_tbl.contains("duration_mappings") ||
      !duration_tbl["duration_mappings"].is_table()) {
    time_tracer::domain::ports::EmitError(
        "[Validator] Error: Duration rules config must contain a "
        "'duration_mappings' table.");
    return false;
  }

  const auto* mappings = duration_tbl["duration_mappings"].as_table();
  for (const auto& [key, node] : *mappings) {
    if (!node.is_array()) {
      time_tracer::domain::ports::EmitError(
          "[Validator] Error: In duration rules, the value for key '" +
          std::string(key.str()) + "' must be an array.");
      return false;
    }

    const auto& rules_array = *node.as_array();
    for (const auto& rule_node : rules_array) {
      if (!rule_node.is_table()) {
        time_tracer::domain::ports::EmitError(
            "[Validator] Error: In duration rules for '" +
            std::string(key.str()) + "', each rule must be an inline table.");
        return false;
      }
      const auto& rule = *rule_node.as_table();

      if (!rule.contains("less_than_minutes") || !rule.contains("value")) {
        time_tracer::domain::ports::EmitError(
            "[Validator] Error: Rules must have 'less_than_minutes' and "
            "'value'.");
        return false;
      }
      if (!rule["less_than_minutes"].is_integer()) {
        time_tracer::domain::ports::EmitError(
            "[Validator] Error: 'less_than_minutes' must be an integer.");
        return false;
      }
      if (!rule["value"].is_string()) {
        time_tracer::domain::ports::EmitError(
            "[Validator] Error: 'value' must be a string.");
        return false;
      }
    }
  }
  return true;
}

auto MappingRule::Validate(const toml::table& mappings_tbl) -> bool {
  if (!mappings_tbl.contains("text_mappings") ||
      !mappings_tbl["text_mappings"].is_table()) {
    time_tracer::domain::ports::EmitError(
        "[Validator] Error: Mappings config must contain a 'text_mappings' "
        "table.");
    return false;
  }
  return true;
}
