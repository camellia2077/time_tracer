// infra/config/loader/converter_config_loader.cpp
#include "infra/config/loader/converter_config_loader.hpp"

#include <algorithm>
#include <cstddef>
#include <filesystem>
#include <stdexcept>
#include <string_view>

#include "infra/config/loader/alias_mapping_index_utils.hpp"
#include "infra/config/validator/converter/rules/converter_rules.hpp"

import tracer.core.infrastructure.config.loader.toml_loader_utils;

namespace fs = std::filesystem;
namespace modloader = tracer::core::infrastructure::config::loader;
namespace modalias = tracer::core::infrastructure::config::loader::detail;

namespace {

auto BuildTextMappingsFromAlias(toml::table& main_tbl,
                                const fs::path& alias_directory_path) -> void {
  const modalias::AliasMappingDefinition definition =
      modalias::LoadAliasMappingDefinition(alias_directory_path,
                                           modloader::ReadToml);
  toml::table text_mappings;
  for (const auto& entry : definition.expanded_entries) {
    text_mappings.insert(entry.alias_key, entry.canonical_value);
  }
  main_tbl.insert_or_assign("text_mappings", std::move(text_mappings));
}

}  // namespace

namespace tracer::core::infrastructure::config {

auto ConverterConfigLoader::LoadMergedToml(const fs::path& main_config_path)
    -> toml::table {
  if (!fs::exists(main_config_path)) {
    throw std::runtime_error("Converter config file not found: " +
                             main_config_path.string());
  }

  toml::table main_tbl = modloader::ReadToml(main_config_path);
  const fs::path alias_directory_path =
      main_config_path.parent_path() / "activity_hierarchy";

  if (!MainRule::Validate(main_tbl)) {
    throw std::runtime_error(
        "Converter config validation failed for main config: " +
        main_config_path.string());
  }

  try {
    // LoadAliasMappingDefinition retains the child TOML path, source
    // location, field path, and remediation hint. Do not reduce that
    // diagnostic to the parent directory; it is the actionable error shown by
    // the CLI and Android runtime.
    static_cast<void>(modalias::LoadAliasMappingDefinition(
        alias_directory_path, modloader::ReadToml));
  } catch (const std::exception& error) {
    throw std::runtime_error(
        "Converter config validation failed for activity hierarchy TOML: " +
        std::string(error.what()));
  }

  BuildTextMappingsFromAlias(main_tbl, alias_directory_path);

  return main_tbl;
}

auto ConverterConfigLoader::ParseTomlToStruct(const toml::table& tbl,
                                              ConverterConfig& config) -> void {
  ParseSleepInference(tbl, config);
  ParseMappings(tbl, config);
}

auto ConverterConfigLoader::ParseSleepInference(const toml::table& tbl,
                                                ConverterConfig& config)
    -> void {
  const toml::table* sleep_inference_tbl =
      tbl["sleep_inference"].as_table();
  if (sleep_inference_tbl == nullptr) {
    throw std::runtime_error(
        "Invalid converter config: 'sleep_inference' must be a table.");
  }

  const toml::array* wake_keywords =
      sleep_inference_tbl->get_as<toml::array>("wake_keywords");
  if (wake_keywords == nullptr || wake_keywords->empty()) {
    throw std::runtime_error(
        "Invalid converter config: 'sleep_inference.wake_keywords' must be a "
        "non-empty array.");
  }
  config.sleep_inference.wake_keywords.clear();
  config.sleep_inference.wake_keywords.reserve(wake_keywords->size());
  for (const auto& elem : *wake_keywords) {
    const auto kValue = elem.value<std::string>();
    if (!kValue || kValue->empty()) {
      throw std::runtime_error(
          "Invalid converter config: each item in "
          "'sleep_inference.wake_keywords' must be a non-empty string.");
    }
    config.sleep_inference.wake_keywords.push_back(*kValue);
  }

  const toml::node* sleep_project_node =
      sleep_inference_tbl->get("sleep_project_path");
  if (sleep_project_node == nullptr) {
    throw std::runtime_error(
        "Invalid converter config: "
        "'sleep_inference.sleep_project_path' must be a non-empty string.");
  }
  const auto kSleepProjectPath = sleep_project_node->value<std::string>();
  if (!kSleepProjectPath || kSleepProjectPath->empty()) {
    throw std::runtime_error(
        "Invalid converter config: "
        "'sleep_inference.sleep_project_path' must be a non-empty string.");
  }
  config.sleep_inference.sleep_project_path = *kSleepProjectPath;
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
}

auto ConverterConfigLoader::LoadFromFile(const fs::path& main_config_path)
    -> ConverterConfig {
  toml::table merged_toml = LoadMergedToml(main_config_path);
  ConverterConfig config;
  ParseTomlToStruct(merged_toml, config);
  return config;
}

}  // namespace tracer::core::infrastructure::config
