// infrastructure/config/validator/converter/rules/converter_rules.cpp
#include "infrastructure/config/validator/converter/rules/converter_rules.hpp"

#include <algorithm>
#include <set>
#include <string>
#include <string_view>

#include "domain/ports/diagnostics.hpp"

namespace {

constexpr std::string_view kKeyDurationRulesPath = "duration_rules_config_path";
constexpr std::string_view kKeyAliasMappingPath = "alias_mapping_path";

auto ValidateMainStrictAlias(const toml::table& main_tbl,
                             MainConfigPaths& out_paths) -> bool {
  const std::set<std::string> kRequiredKeys = {
      "alias_mapping_path", "duration_rules_config_path", "header_order",
      "remark_prefix", "wake_keywords"};

  for (const auto& key : kRequiredKeys) {
    if (!main_tbl.contains(key)) {
      tracer_core::domain::ports::EmitError(
          "[Validator] Error: Main config is missing required key: '" + key +
          "'");
      return false;
    }
  }

  if (main_tbl.contains("mappings_config_path")) {
    tracer_core::domain::ports::EmitError(
        "[Validator] Error: 'mappings_config_path' is no longer supported. "
        "Use 'alias_mapping_path' and alias_mapping.toml [aliases].");
    return false;
  }

  if (!main_tbl[kKeyAliasMappingPath].is_string() ||
      !main_tbl[kKeyDurationRulesPath].is_string()) {
    tracer_core::domain::ports::EmitError(
        "[Validator] Error: 'alias_mapping_path' and "
        "'duration_rules_config_path' must be strings.");
    return false;
  }
  if (!main_tbl["header_order"].is_array() ||
      !main_tbl["wake_keywords"].is_array()) {
    tracer_core::domain::ports::EmitError(
        "[Validator] Error: 'header_order' and 'wake_keywords' must be "
        "arrays.");
    return false;
  }
  if (!main_tbl["remark_prefix"].is_string()) {
    tracer_core::domain::ports::EmitError(
        "[Validator] Error: 'remark_prefix' must be a string.");
    return false;
  }
  if (main_tbl.contains("top_parent_mapping") &&
      !main_tbl["top_parent_mapping"].is_table()) {
    tracer_core::domain::ports::EmitError(
        "[Validator] Error: 'top_parent_mapping' must be a table when "
        "present.");
    return false;
  }

  const auto kAliasMappingPath =
      main_tbl[kKeyAliasMappingPath].value<std::string>();
  const auto kDurationRulesPath =
      main_tbl[kKeyDurationRulesPath].value<std::string>();
  if (!kAliasMappingPath.has_value() || !kDurationRulesPath.has_value()) {
    tracer_core::domain::ports::EmitError(
        "[Validator] Error: 'alias_mapping_path' and "
        "'duration_rules_config_path' must be strings.");
    return false;
  }

  out_paths.alias_mapping_path = *kAliasMappingPath;
  out_paths.duration_rules_config_path = *kDurationRulesPath;
  return true;
}

}  // namespace

auto MainRule::Validate(const toml::table& main_tbl, MainConfigPaths& out_paths)
    -> bool {
  return ValidateMainStrictAlias(main_tbl, out_paths);
}

auto DurationRule::Validate(const toml::table& duration_tbl) -> bool {
  if (!duration_tbl.contains("text_duration_mappings") ||
      !duration_tbl["text_duration_mappings"].is_table()) {
    tracer_core::domain::ports::EmitError(
        "[Validator] Error: Duration rules config must contain a "
        "'text_duration_mappings' table.");
    return false;
  }

  if (!duration_tbl.contains("duration_mappings") ||
      !duration_tbl["duration_mappings"].is_table()) {
    tracer_core::domain::ports::EmitError(
        "[Validator] Error: Duration rules config must contain a "
        "'duration_mappings' table.");
    return false;
  }

  const auto* mappings = duration_tbl["duration_mappings"].as_table();
  for (const auto& [key, node] : *mappings) {
    if (!node.is_array()) {
      tracer_core::domain::ports::EmitError(
          "[Validator] Error: In duration rules, the value for key '" +
          std::string(key.str()) + "' must be an array.");
      return false;
    }

    const auto& rules_array = *node.as_array();
    for (const auto& rule_node : rules_array) {
      if (!rule_node.is_table()) {
        tracer_core::domain::ports::EmitError(
            "[Validator] Error: In duration rules for '" +
            std::string(key.str()) + "', each rule must be an inline table.");
        return false;
      }
      const auto& rule = *rule_node.as_table();

      if (!rule.contains("less_than_minutes") || !rule.contains("value")) {
        tracer_core::domain::ports::EmitError(
            "[Validator] Error: Rules must have 'less_than_minutes' and "
            "'value'.");
        return false;
      }
      if (!rule["less_than_minutes"].is_integer()) {
        tracer_core::domain::ports::EmitError(
            "[Validator] Error: 'less_than_minutes' must be an integer.");
        return false;
      }
      if (!rule["value"].is_string()) {
        tracer_core::domain::ports::EmitError(
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
    tracer_core::domain::ports::EmitError(
        "[Validator] Error: Mappings config must contain a 'text_mappings' "
        "table.");
    return false;
  }
  return true;
}

auto V2Rule::ValidateAliasMapping(const toml::table& alias_tbl) -> bool {
  const toml::table* aliases = alias_tbl["aliases"].as_table();
  if (aliases == nullptr) {
    tracer_core::domain::ports::EmitError(
        "[Validator] Error: 'aliases' must be a table.");
    return false;
  }
  return std::ranges::all_of(*aliases, [](const auto& alias_entry) -> bool {
    const auto& [alias_key, node] = alias_entry;
    const std::string kAliasKey = std::string(alias_key.str());
    if (kAliasKey.empty()) {
      tracer_core::domain::ports::EmitError(
          "[Validator] Error: aliases contains an empty key.");
      return false;
    }
    const auto kAliasValue = node.template value<std::string>();
    if (!kAliasValue || kAliasValue->empty()) {
      tracer_core::domain::ports::EmitError("[Validator] Error: aliases.'" +
                                            kAliasKey +
                                            "' must be a non-empty string.");
      return false;
    }
    return true;
  });
}
