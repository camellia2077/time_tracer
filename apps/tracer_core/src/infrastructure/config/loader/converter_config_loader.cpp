// infrastructure/config/loader/converter_config_loader.cpp
#include "infrastructure/config/loader/converter_config_loader.hpp"

#include <algorithm>
#include <cstddef>
#include <stdexcept>
#include <string_view>

#include "infrastructure/config/loader/toml_loader_utils.hpp"  // 使用 read_toml
#include "infrastructure/config/validator/converter/rules/converter_rules.hpp"
#include "infrastructure/io/core/file_system_helper.hpp"

namespace fs = std::filesystem;
using namespace TomlLoaderUtils;

namespace {

constexpr std::string_view kAliasesSection = "aliases";

auto ReadRequiredToml(const fs::path& file_path, std::string_view logical_name)
    -> toml::table {
  if (!FileSystemHelper::Exists(file_path)) {
    throw std::runtime_error(std::string(logical_name) +
                             " config file not found: " + file_path.string());
  }
  return ReadToml(file_path);
}

auto BuildTextMappingsFromAlias(toml::table& main_tbl,
                                const toml::table& alias_tbl) -> void {
  const toml::table* aliases = alias_tbl[kAliasesSection].as_table();

  toml::table text_mappings;
  for (const auto& [alias_key, alias_value] : *aliases) {
    const std::string kAlias = std::string(alias_key.str());
    const auto kProjectPathNode = alias_value.value<std::string>();
    if (!kProjectPathNode.has_value()) {
      throw std::runtime_error(
          "aliases values must be non-empty strings in alias mapping config.");
    }
    const std::string kProjectPath = *kProjectPathNode;
    text_mappings.insert(kAlias, kProjectPath);
  }
  main_tbl.insert_or_assign("text_mappings", std::move(text_mappings));
}

}  // namespace

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

  MainConfigPaths paths;
  if (!MainRule::Validate(main_tbl, paths)) {
    throw std::runtime_error(
        "Converter config validation failed for main config: " +
        main_config_path.string());
  }

  const fs::path kAliasMappingPath = config_dir / paths.alias_mapping_path;
  const fs::path kDurationRulesPath =
      config_dir / paths.duration_rules_config_path;

  const toml::table kAliasMappingTbl =
      ReadRequiredToml(kAliasMappingPath, "Alias mapping");
  const toml::table kDurationTbl =
      ReadRequiredToml(kDurationRulesPath, "Duration rules");
  if (!V2Rule::ValidateAliasMapping(kAliasMappingTbl) ||
      !DurationRule::Validate(kDurationTbl)) {
    throw std::runtime_error(
        "Converter config validation failed for converter schema under: " +
        config_dir.string());
  }

  BuildTextMappingsFromAlias(main_tbl, kAliasMappingTbl);
  MergeSectionIfPresent(main_tbl, kDurationTbl, "text_duration_mappings");
  MergeSectionIfPresent(main_tbl, kDurationTbl, "duration_mappings");

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
  const auto kRemarkPrefix = tbl["remark_prefix"].value<std::string>();
  if (!kRemarkPrefix || kRemarkPrefix->empty()) {
    throw std::runtime_error(
        "Invalid converter config: 'remark_prefix' must be a non-empty "
        "string.");
  }
  config.remark_prefix = *kRemarkPrefix;

  const toml::array* header_order = tbl["header_order"].as_array();
  if (header_order == nullptr || header_order->empty()) {
    throw std::runtime_error(
        "Invalid converter config: 'header_order' must be a non-empty array.");
  }
  config.header_order.clear();
  config.header_order.reserve(header_order->size());
  for (const auto& elem : *header_order) {
    const auto kValue = elem.value<std::string>();
    if (!kValue || kValue->empty()) {
      throw std::runtime_error(
          "Invalid converter config: each item in 'header_order' must be a "
          "non-empty string.");
    }
    config.header_order.push_back(*kValue);
  }

  const toml::array* wake_keywords = tbl["wake_keywords"].as_array();
  if (wake_keywords == nullptr || wake_keywords->empty()) {
    throw std::runtime_error(
        "Invalid converter config: 'wake_keywords' must be a non-empty array.");
  }
  config.wake_keywords.clear();
  config.wake_keywords.reserve(wake_keywords->size());
  for (const auto& elem : *wake_keywords) {
    const auto kValue = elem.value<std::string>();
    if (!kValue || kValue->empty()) {
      throw std::runtime_error(
          "Invalid converter config: each item in 'wake_keywords' must be a "
          "non-empty string.");
    }
    config.wake_keywords.push_back(*kValue);
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
      const auto kSleepProjectPath = sleep_project_node->value<std::string>();
      if (!kSleepProjectPath || kSleepProjectPath->empty()) {
        throw std::runtime_error(
            "Invalid converter config: "
            "'generated_activities.sleep_project_path' "
            "must be a non-empty string.");
      }
      config.generated_sleep_project_path = *kSleepProjectPath;
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
      const std::string kEntryKey = std::string(k.str());
      if (kEntryKey.empty()) {
        throw std::runtime_error("Invalid converter config: '" + key +
                                 "' contains an empty key.");
      }

      const auto kEntryValue = v.value<std::string>();
      if (!kEntryValue || kEntryValue->empty()) {
        throw std::runtime_error("Invalid converter config: value of '" + key +
                                 "." + kEntryKey +
                                 "' must be a non-empty string.");
      }
      target[kEntryKey] = *kEntryValue;
    }
  };

  if (tbl.contains("top_parent_mapping")) {
    load_map("top_parent_mapping", config.top_parent_mapping);
  } else {
    config.top_parent_mapping.clear();
  }
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
    const std::string kEvent = std::string(event_key.str());
    if (kEvent.empty()) {
      throw std::runtime_error(
          "Invalid converter config: 'duration_mappings' contains an empty "
          "key.");
    }

    const toml::array* rules_arr = rules_node.as_array();
    if (rules_arr == nullptr || rules_arr->empty()) {
      throw std::runtime_error("Invalid converter config: 'duration_mappings." +
                               kEvent + "' must be a non-empty array.");
    }

    std::vector<DurationMappingRule> rules;
    rules.reserve(rules_arr->size());
    for (const auto& rule_node : *rules_arr) {
      const toml::table* rule_tbl = rule_node.as_table();
      if (rule_tbl == nullptr) {
        throw std::runtime_error(
            "Invalid converter config: each rule in 'duration_mappings." +
            kEvent + "' must be a table.");
      }

      const auto kLessThanMinutes =
          rule_tbl->get("less_than_minutes")->value<int>();
      const auto kValue = rule_tbl->get("value")->value<std::string>();
      if (!kLessThanMinutes || *kLessThanMinutes <= 0) {
        throw std::runtime_error(
            "Invalid converter config: 'less_than_minutes' in "
            "'duration_mappings." +
            kEvent + "' must be a positive integer.");
      }
      if (!kValue || kValue->empty()) {
        throw std::runtime_error(
            "Invalid converter config: 'value' in 'duration_mappings." +
            kEvent + "' must be a non-empty string.");
      }

      rules.push_back(DurationMappingRule{
          .less_than_minutes = *kLessThanMinutes,
          .value = *kValue,
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
            kEvent + "' must be strictly increasing.");
      }
    }

    config.duration_mappings[kEvent] = std::move(rules);
  }
}

auto ConverterConfigLoader::LoadFromFile(const fs::path& main_config_path)
    -> ConverterConfig {
  toml::table merged_toml = LoadMergedToml(main_config_path);
  ConverterConfig config;
  ParseTomlToStruct(merged_toml, config);
  return config;
}
