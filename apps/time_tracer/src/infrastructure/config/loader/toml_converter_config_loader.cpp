// infrastructure/config/loader/toml_converter_config_loader.cpp
#include "infrastructure/config/loader/toml_converter_config_loader.hpp"

#include <algorithm>
#include <iostream>

#include "shared/types/ansi_colors.hpp"

TomlConverterConfigLoader::TomlConverterConfigLoader(
    const toml::table& config_table)
    : toml_source_(config_table) {}

auto TomlConverterConfigLoader::Load(ConverterConfig& config) -> bool {
  try {
    // remark_prefix
    if (auto val = toml_source_["remark_prefix"].value<std::string>()) {
      config.remark_prefix = *val;
    }

    parse_header_order(config);
    parse_wake_keywords(config);
    parse_top_parent_mapping(config);
    parse_text_mappings(config);
    parse_text_duration_mappings(config);
    parse_duration_mappings(config);

  } catch (const std::exception& e) {
    namespace colors = time_tracer::common::colors;
    std::cerr << colors::kRed
              << "Error parsing configuration TOML via Loader: " << e.what()
              << colors::kReset << std::endl;
    return false;
  }
  return true;
}

void TomlConverterConfigLoader::parse_header_order(ConverterConfig& config) {
  if (const toml::array* arr = toml_source_["header_order"].as_array()) {
    config.header_order.clear();
    for (const auto& elem : *arr) {
      if (auto val_str = elem.value<std::string>()) {
        config.header_order.push_back(*val_str);
      }
    }
  }
}

void TomlConverterConfigLoader::parse_wake_keywords(ConverterConfig& config) {
  if (const toml::array* arr = toml_source_["wake_keywords"].as_array()) {
    config.wake_keywords.clear();
    for (const auto& elem : *arr) {
      if (auto val_str = elem.value<std::string>()) {
        config.wake_keywords.push_back(*val_str);
      }
    }
  }
}

void TomlConverterConfigLoader::parse_top_parent_mapping(
    ConverterConfig& config) {
  if (const toml::table* tbl = toml_source_["top_parent_mapping"].as_table()) {
    for (const auto& [k, v] : *tbl) {
      if (auto val_str = v.value<std::string>()) {
        config.top_parent_mapping[std::string(k.str())] = *val_str;
      }
    }
  }
}

void TomlConverterConfigLoader::parse_text_mappings(ConverterConfig& config) {
  if (const toml::table* tbl = toml_source_["text_mappings"].as_table()) {
    for (const auto& [k, v] : *tbl) {
      if (auto val_str = v.value<std::string>()) {
        config.text_mapping[std::string(k.str())] = *val_str;
      }
    }
  }
}

void TomlConverterConfigLoader::parse_text_duration_mappings(
    ConverterConfig& config) {
  if (const toml::table* tbl =
          toml_source_["text_duration_mappings"].as_table()) {
    for (const auto& [k, v] : *tbl) {
      if (auto val_str = v.value<std::string>()) {
        config.text_duration_mapping[std::string(k.str())] = *val_str;
      }
    }
  }
}

void TomlConverterConfigLoader::parse_duration_mappings(
    ConverterConfig& config) {
  if (const toml::table* duration_tbl =
          toml_source_["duration_mappings"].as_table()) {
    for (const auto& [event_key, rules_node] : *duration_tbl) {
      if (const toml::array* rules_arr = rules_node.as_array()) {
        std::vector<DurationMappingRule> rules;

        for (const auto& rule_node : *rules_arr) {
          if (const toml::table* rule_tbl = rule_node.as_table()) {
            DurationMappingRule rule;
            rule.less_than_minutes =
                rule_tbl->get("less_than_minutes")->value_or(0);
            rule.value = rule_tbl->get("value")->value_or("");
            rules.push_back(rule);
          }
        }
        std::ranges::sort(rules,
                          [](const DurationMappingRule& rule_a,
                             const DurationMappingRule& rule_b) -> bool {
                            return rule_a.less_than_minutes <
                                   rule_b.less_than_minutes;
                          });

        config.duration_mappings[std::string(event_key.str())] = rules;
      }
    }
  }
}