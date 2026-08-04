#include <nlohmann/json.hpp>

#include <filesystem>
#include <optional>
#include <set>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "infra/config/loader/alias_mapping_index_utils.hpp"
#include "infra/config/loader/toml_loader_utils.hpp"
#include "infra/query/data/internal/report_mapping.hpp"

import tracer.core.infrastructure.config.file_converter_config_provider;
import tracer.core.domain.types.converter_config;

namespace modtypes = tracer::core::domain::modtypes;
namespace modloader = tracer::core::infrastructure::config::loader;
using FileConverterConfigProvider =
    tracer::core::infrastructure::config::FileConverterConfigProvider;

namespace tracer::core::infrastructure::query::data::internal {
namespace {

using nlohmann::json;

auto LoadConverterConfigOrThrow(
    const std::optional<std::filesystem::path>& converter_config_toml_path,
    std::string_view query_name) -> modtypes::ConverterConfig {
  if (!converter_config_toml_path.has_value() ||
      converter_config_toml_path->empty()) {
    throw std::runtime_error(std::string(query_name) +
                             " query requires converter config path.");
  }

  FileConverterConfigProvider config_provider(
      *converter_config_toml_path,
      std::unordered_map<std::filesystem::path, std::filesystem::path>{});
  return config_provider.LoadConverterConfig();
}

auto BuildNamesPayload(const std::set<std::string>& names) -> std::string {
  json payload = json::object();
  payload["names"] = json::array();
  for (const auto& name : names) {
    payload["names"].push_back(name);
  }
  return payload.dump();
}

auto BuildAliasEntriesPayload(
    const std::vector<modloader::detail::ExpandedAliasMappingEntry>& entries)
    -> std::string {
  json payload = json::object();
  payload["entries"] = json::array();
  for (const auto& entry : entries) {
    payload["entries"].push_back(json{
        {"alias", entry.alias_key},
        {"canonical", entry.canonical_value},
    });
  }
  return payload.dump();
}

}  // namespace

auto BuildMappingNamesContent(
    const std::optional<std::filesystem::path>& converter_config_toml_path)
    -> std::string {
  const modtypes::ConverterConfig kConfig =
      LoadConverterConfigOrThrow(converter_config_toml_path, "mapping_names");

  std::set<std::string> names;
  for (const auto& [alias, full_name] : kConfig.text_mapping) {
    const std::string kTrimmedAlias = TrimCopy(alias);
    const std::string kTrimmedFullName = TrimCopy(full_name);
    if (!kTrimmedAlias.empty()) {
      names.insert(kTrimmedAlias);
    }
    if (!kTrimmedFullName.empty()) {
      names.insert(kTrimmedFullName);
    }
  }
  return BuildNamesPayload(names);
}

auto BuildActivityAliasMappingsContent(
    const std::optional<std::filesystem::path>& converter_config_toml_path)
    -> std::string {
  if (!converter_config_toml_path.has_value() ||
      converter_config_toml_path->empty()) {
    throw std::runtime_error(
        "activity_alias_mappings query requires converter config path.");
  }

  const std::filesystem::path kConfigPath = *converter_config_toml_path;
  const std::filesystem::path kConfigDir = kConfigPath.parent_path();
  const auto kDefinition = modloader::detail::LoadAliasMappingDefinition(
      kConfigDir / "activity_hierarchy", [](const std::filesystem::path& path) {
        return modloader::ReadToml(path);
      });
  return BuildAliasEntriesPayload(kDefinition.expanded_entries);
}

auto BuildMappingAliasKeysContent(
    const std::optional<std::filesystem::path>& converter_config_toml_path)
    -> std::string {
  const modtypes::ConverterConfig kConfig = LoadConverterConfigOrThrow(
      converter_config_toml_path, "mapping_alias_keys");

  std::set<std::string> alias_keys;
  for (const auto& [alias, full_name] : kConfig.text_mapping) {
    static_cast<void>(full_name);
    const std::string kTrimmedAlias = TrimCopy(alias);
    if (!kTrimmedAlias.empty()) {
      alias_keys.insert(kTrimmedAlias);
    }
  }
  return BuildNamesPayload(alias_keys);
}

auto BuildWakeKeywordsContent(
    const std::optional<std::filesystem::path>& converter_config_toml_path)
    -> std::string {
  const modtypes::ConverterConfig kConfig =
      LoadConverterConfigOrThrow(converter_config_toml_path, "wake_keywords");

  std::set<std::string> wake_keywords;
  for (const auto& wake_keyword : kConfig.sleep_inference.wake_keywords) {
    const std::string kTrimmedKeyword = TrimCopy(wake_keyword);
    if (!kTrimmedKeyword.empty()) {
      wake_keywords.insert(kTrimmedKeyword);
    }
  }
  return BuildNamesPayload(wake_keywords);
}

auto BuildAuthorableEventTokensContent(
    const std::optional<std::filesystem::path>& converter_config_toml_path)
    -> std::string {
  const modtypes::ConverterConfig kConfig = LoadConverterConfigOrThrow(
      converter_config_toml_path, "authorable_event_tokens");

  std::set<std::string> authorable_tokens;
  for (const auto& [alias, full_name] : kConfig.text_mapping) {
    const std::string kTrimmedAlias = TrimCopy(alias);
    const std::string kTrimmedCanonical = TrimCopy(full_name);
    if (!kTrimmedAlias.empty()) {
      authorable_tokens.insert(kTrimmedAlias);
    }
    if (!kTrimmedCanonical.empty()) {
      authorable_tokens.insert(kTrimmedCanonical);
    }
  }
  for (const auto& wake_keyword : kConfig.sleep_inference.wake_keywords) {
    const std::string kTrimmedKeyword = TrimCopy(wake_keyword);
    if (!kTrimmedKeyword.empty()) {
      authorable_tokens.insert(kTrimmedKeyword);
    }
  }
  return BuildNamesPayload(authorable_tokens);
}

}  // namespace tracer::core::infrastructure::query::data::internal
