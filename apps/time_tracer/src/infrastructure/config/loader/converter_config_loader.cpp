// infrastructure/config/loader/converter_config_loader.cpp
#include "infrastructure/config/loader/converter_config_loader.hpp"

#include <algorithm>
#include <cstddef>
#include <stdexcept>

#include "infrastructure/config/loader/toml_loader_utils.hpp"  // 使用 read_toml
#include "infrastructure/config/validator/converter/rules/converter_rules.hpp"
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
    toml::table& main_tbl, const fs::path& config_dir,
    std::string_view path_key,
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

  std::string mappings_rel_path;
  std::string duration_rules_rel_path;
  if (!MainRule::Validate(main_tbl, mappings_rel_path,
                          duration_rules_rel_path)) {
    throw std::runtime_error(
        "Converter config validation failed for main config: " +
        main_config_path.string());
  }

  const fs::path mappings_path = config_dir / mappings_rel_path;
  if (!FileSystemHelper::Exists(mappings_path)) {
    throw std::runtime_error("Mappings config file not found: " +
                             mappings_path.string());
  }
  const toml::table mappings_tbl = ReadToml(mappings_path);
  if (!MappingRule::Validate(mappings_tbl)) {
    throw std::runtime_error(
        "Converter config validation failed for mappings: " +
        mappings_path.string());
  }
  MergeSectionIfPresent(main_tbl, mappings_tbl, "text_mappings");
  MergeSectionIfPresent(main_tbl, mappings_tbl, "text_duration_mappings");

  const fs::path duration_rules_path = config_dir / duration_rules_rel_path;
  if (!FileSystemHelper::Exists(duration_rules_path)) {
    throw std::runtime_error("Duration rules config file not found: " +
                             duration_rules_path.string());
  }
  const toml::table duration_tbl = ReadToml(duration_rules_path);
  if (!DurationRule::Validate(duration_tbl)) {
    throw std::runtime_error(
        "Converter config validation failed for duration rules: " +
        duration_rules_path.string());
  }
  MergeSectionIfPresent(main_tbl, duration_tbl, "duration_mappings");
  MergeSectionIfPresent(main_tbl, duration_tbl, "text_duration_mappings");

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
  const auto remark_prefix = tbl["remark_prefix"].value<std::string>();
  if (!remark_prefix || remark_prefix->empty()) {
    throw std::runtime_error(
        "Invalid converter config: 'remark_prefix' must be a non-empty "
        "string.");
  }
  config.remark_prefix = *remark_prefix;

  const toml::array* header_order = tbl["header_order"].as_array();
  if (header_order == nullptr || header_order->empty()) {
    throw std::runtime_error(
        "Invalid converter config: 'header_order' must be a non-empty array.");
  }
  config.header_order.clear();
  config.header_order.reserve(header_order->size());
  for (const auto& elem : *header_order) {
    const auto value = elem.value<std::string>();
    if (!value || value->empty()) {
      throw std::runtime_error(
          "Invalid converter config: each item in 'header_order' must be a "
          "non-empty string.");
    }
    config.header_order.push_back(*value);
  }

  const toml::array* wake_keywords = tbl["wake_keywords"].as_array();
  if (wake_keywords == nullptr || wake_keywords->empty()) {
    throw std::runtime_error(
        "Invalid converter config: 'wake_keywords' must be a non-empty array.");
  }
  config.wake_keywords.clear();
  config.wake_keywords.reserve(wake_keywords->size());
  for (const auto& elem : *wake_keywords) {
    const auto value = elem.value<std::string>();
    if (!value || value->empty()) {
      throw std::runtime_error(
          "Invalid converter config: each item in 'wake_keywords' must be a "
          "non-empty string.");
    }
    config.wake_keywords.push_back(*value);
  }
}

auto ConverterConfigLoader::ParseGeneratedActivities(const toml::table& tbl,
                                                     ConverterConfig& config)
    -> void {
  if (tbl.contains("generated_activities")) {
    const toml::table* gen_tbl = tbl["generated_activities"].as_table();
    if (gen_tbl == nullptr) {
      throw std::runtime_error(
          "Invalid converter config: 'generated_activities' must be a table.");
    }

    if (const toml::node* sleep_project_node =
            gen_tbl->get("sleep_project_path")) {
      const auto sleep_project_path = sleep_project_node->value<std::string>();
      if (!sleep_project_path || sleep_project_path->empty()) {
        throw std::runtime_error(
            "Invalid converter config: "
            "'generated_activities.sleep_project_path' "
            "must be a non-empty string.");
      }
      config.generated_sleep_project_path = *sleep_project_path;
    }
  }
}

auto ConverterConfigLoader::ParseMappings(const toml::table& tbl,
                                          ConverterConfig& config) -> void {
  auto load_map =
      [&](const std::string& key,
          std::unordered_map<std::string, std::string>& target) -> void {
    const toml::table* map_tbl = tbl[key].as_table();
    if (map_tbl == nullptr) {
      throw std::runtime_error("Invalid converter config: '" + key +
                               "' must be a table.");
    }

    target.clear();
    for (const auto& [k, v] : *map_tbl) {
      const std::string entry_key = std::string(k.str());
      if (entry_key.empty()) {
        throw std::runtime_error("Invalid converter config: '" + key +
                                 "' contains an empty key.");
      }

      const auto entry_value = v.value<std::string>();
      if (!entry_value || entry_value->empty()) {
        throw std::runtime_error("Invalid converter config: value of '" + key +
                                 "." + entry_key +
                                 "' must be a non-empty string.");
      }
      target[entry_key] = *entry_value;
    }
  };

  load_map("top_parent_mapping", config.top_parent_mapping);
  load_map("text_mappings", config.text_mapping);
  load_map("text_duration_mappings", config.text_duration_mapping);
}

auto ConverterConfigLoader::ParseDurationMappings(const toml::table& tbl,
                                                  ConverterConfig& config)
    -> void {
  const toml::table* duration_tbl = tbl["duration_mappings"].as_table();
  if (duration_tbl == nullptr) {
    throw std::runtime_error(
        "Invalid converter config: 'duration_mappings' must be a table.");
  }

  config.duration_mappings.clear();
  for (const auto& [event_key, rules_node] : *duration_tbl) {
    const std::string event = std::string(event_key.str());
    if (event.empty()) {
      throw std::runtime_error(
          "Invalid converter config: 'duration_mappings' contains an empty "
          "key.");
    }

    const toml::array* rules_arr = rules_node.as_array();
    if (rules_arr == nullptr || rules_arr->empty()) {
      throw std::runtime_error("Invalid converter config: 'duration_mappings." +
                               event + "' must be a non-empty array.");
    }

    std::vector<DurationMappingRule> rules;
    rules.reserve(rules_arr->size());
    for (const auto& rule_node : *rules_arr) {
      const toml::table* rule_tbl = rule_node.as_table();
      if (rule_tbl == nullptr) {
        throw std::runtime_error(
            "Invalid converter config: each rule in 'duration_mappings." +
            event + "' must be a table.");
      }

      const auto less_than_minutes =
          rule_tbl->get("less_than_minutes")->value<int>();
      const auto value = rule_tbl->get("value")->value<std::string>();
      if (!less_than_minutes || *less_than_minutes <= 0) {
        throw std::runtime_error(
            "Invalid converter config: 'less_than_minutes' in "
            "'duration_mappings." +
            event + "' must be a positive integer.");
      }
      if (!value || value->empty()) {
        throw std::runtime_error(
            "Invalid converter config: 'value' in 'duration_mappings." + event +
            "' must be a non-empty string.");
      }

      rules.push_back(DurationMappingRule{
          .less_than_minutes = *less_than_minutes,
          .value = *value,
      });
    }

    std::ranges::sort(rules,
                      [](const DurationMappingRule& rule_a,
                         const DurationMappingRule& rule_b) -> bool {
                        return rule_a.less_than_minutes <
                               rule_b.less_than_minutes;
                      });

    for (size_t index = 1; index < rules.size(); ++index) {
      if (rules[index - 1].less_than_minutes >=
          rules[index].less_than_minutes) {
        throw std::runtime_error(
            "Invalid converter config: 'less_than_minutes' in "
            "'duration_mappings." +
            event + "' must be strictly increasing.");
      }
    }

    config.duration_mappings[event] = std::move(rules);
  }
}

auto ConverterConfigLoader::LoadFromFile(const fs::path& main_config_path)
    -> ConverterConfig {
  toml::table merged_toml = LoadMergedToml(main_config_path);
  ConverterConfig config;
  ParseTomlToStruct(merged_toml, config);
  return config;
}
