// config/loader/converter_config_loader.cpp
#include "config/loader/converter_config_loader.hpp"

#include <algorithm>
#include <iostream>
#include <stdexcept>

#include "common/ansi_colors.hpp"
#include "config/loader/toml_loader_utils.hpp"  // 使用 read_toml
#include "infrastructure/io/core/file_system_helper.hpp"

namespace fs = std::filesystem;
using namespace TomlLoaderUtils;

auto ConverterConfigLoader::MergeTomlTable(toml::table& target,
                                          const toml::table& source) -> void {
  for (const auto& [key, val] : source) {
    if (target.contains(key)) {
      if (target[key].is_table() && val.is_table()) {
        MergeTomlTable(*target[key].as_table(), *val.as_table());
      } else {
        target.insert_or_assign(key, val);
      }
    } else {
      target.insert(key, val);
    }
  }
}

auto ConverterConfigLoader::MergeSectionIfPresent(toml::table& main_tbl,
                                                  const toml::table& source_tbl,
                                                  std::string_view section_key)
    -> void {
  const toml::node_view<const toml::node> kSourceNode = source_tbl[section_key];
  if (!kSourceNode || !kSourceNode.is_table()) {
    return;
  }

  if (!main_tbl.contains(section_key)) {
    main_tbl.insert(section_key, toml::table{});
  }

  MergeTomlTable(*main_tbl[section_key].as_table(), *kSourceNode.as_table());
}

auto ConverterConfigLoader::MergeOptionalSections(
    toml::table& main_tbl, const fs::path& config_dir, std::string_view path_key,
    std::initializer_list<std::string_view> section_keys) -> void {
  auto path_node = main_tbl[path_key].value<std::string>();
  if (!path_node) {
    return;
  }

  fs::path file_path = config_dir / *path_node;
  if (!FileSystemHelper::Exists(file_path)) {
    return;
  }

  toml::table source_tbl = ReadToml(file_path);
  for (std::string_view section_key : section_keys) {
    MergeSectionIfPresent(main_tbl, source_tbl, section_key);
  }
}

auto ConverterConfigLoader::LoadMergedToml(const fs::path& main_config_path)
    -> toml::table {
  if (!FileSystemHelper::Exists(main_config_path)) {
    throw std::runtime_error("Converter config file not found: " +
                             main_config_path.string());
  }

  toml::table main_tbl = ReadToml(main_config_path);
  fs::path config_dir = main_config_path.parent_path();

  MergeOptionalSections(main_tbl, config_dir, "mappings_config_path",
                        {"text_mappings", "text_duration_mappings"});

  MergeOptionalSections(main_tbl, config_dir, "duration_rules_config_path",
                        {"duration_mappings", "text_duration_mappings"});

  return main_tbl;
}

auto ConverterConfigLoader::ParseTomlToStruct(const toml::table& tbl,
                                              ConverterConfig& config) -> void {
  ParseBasicConfig(tbl, config);
  ParseGeneratedActivities(tbl, config);
  ParseMappings(tbl, config);
  ParseDurationMappings(tbl, config);
}

auto ConverterConfigLoader::ParseBasicConfig(const toml::table& tbl,
                                             ConverterConfig& config) -> void {

  if (auto val = tbl["remark_prefix"].value<std::string>()) {
    config.remark_prefix = *val;
  }

  if (const toml::array* arr = tbl["header_order"].as_array()) {
    for (const auto& elem : *arr) {
      if (auto val_str = elem.value<std::string>()) {
        config.header_order.push_back(*val_str);
      }
    }
  }

  if (const toml::array* arr = tbl["wake_keywords"].as_array()) {
    for (const auto& elem : *arr) {
      if (auto val_str = elem.value<std::string>()) {
        config.wake_keywords.push_back(*val_str);
      }
    }
  }
}

auto ConverterConfigLoader::ParseGeneratedActivities(
    const toml::table& tbl, ConverterConfig& config) -> void {
  if (const toml::table* gen_tbl = tbl["generated_activities"].as_table()) {
    if (auto val = gen_tbl->get("sleep_project_path")->value<std::string>()) {
      config.generated_sleep_project_path = *val;
    }
  }
}

auto ConverterConfigLoader::ParseMappings(const toml::table& tbl,
                                          ConverterConfig& config) -> void {

  auto load_map =
      [&](const std::string& key,
          std::unordered_map<std::string, std::string>& target) -> void {
    if (const toml::table* map_tbl = tbl[key].as_table()) {
      for (const auto& [k, v] : *map_tbl) {
        if (auto val_str = v.value<std::string>()) {
          target[std::string(k.str())] = *val_str;
        }
      }
    }
  };

  load_map("top_parent_mapping", config.top_parent_mapping);
  load_map("text_mappings", config.text_mapping);
  load_map("text_duration_mappings", config.text_duration_mapping);
}

auto ConverterConfigLoader::ParseDurationMappings(const toml::table& tbl,
                                                   ConverterConfig& config)
    -> void {
  if (const toml::table* duration_tbl = tbl["duration_mappings"].as_table()) {
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

auto ConverterConfigLoader::LoadFromFile(const fs::path& main_config_path)
    -> ConverterConfig {
  toml::table merged_toml = LoadMergedToml(main_config_path);
  ConverterConfig config;
  ParseTomlToStruct(merged_toml, config);
  return config;
}

