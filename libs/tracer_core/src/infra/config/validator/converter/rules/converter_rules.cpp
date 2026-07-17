// infra/config/validator/converter/rules/converter_rules.cpp
#include "infra/config/validator/converter/rules/converter_rules.hpp"

#include <algorithm>
#include <filesystem>
#include <set>
#include <string>
#include <string_view>

#include "infra/config/loader/alias_mapping_index_utils.hpp"
#include "infra/config/loader/toml_loader_utils.hpp"

import tracer.core.domain.ports.diagnostics;

namespace modports = tracer::core::domain::ports;
namespace modloader = tracer::core::infrastructure::config::loader;
namespace modalias = tracer::core::infrastructure::config::loader::detail;
namespace fs = std::filesystem;

namespace {

constexpr std::string_view kKeyAliasMappingPath = "alias_mapping_path";

auto ValidateMainStrictAlias(const toml::table& main_tbl,
                             MainConfigPaths& out_paths) -> bool {
  const std::set<std::string> kRequiredKeys = {
      "alias_mapping_path", "header_order",
      "remark_prefix", "wake_keywords"};

  for (const auto& key : kRequiredKeys) {
    if (!main_tbl.contains(key)) {
      modports::EmitError(
          "[Validator] Error: Main config is missing required key: '" + key +
          "'");
      return false;
    }
  }

  if (main_tbl.contains("mappings_config_path")) {
    modports::EmitError(
        "[Validator] Error: 'mappings_config_path' is no longer supported. "
        "Use 'alias_mapping_path' and alias mapping index files.");
    return false;
  }

  if (!main_tbl[kKeyAliasMappingPath].is_string()) {
    modports::EmitError("[Validator] Error: 'alias_mapping_path' must be a string.");
    return false;
  }
  if (!main_tbl["header_order"].is_array() ||
      !main_tbl["wake_keywords"].is_array()) {
    modports::EmitError(
        "[Validator] Error: 'header_order' and 'wake_keywords' must be "
        "arrays.");
    return false;
  }
  if (!main_tbl["remark_prefix"].is_string()) {
    modports::EmitError("[Validator] Error: 'remark_prefix' must be a string.");
    return false;
  }
  if (main_tbl.contains("top_parent_mapping") &&
      !main_tbl["top_parent_mapping"].is_table()) {
    modports::EmitError(
        "[Validator] Error: 'top_parent_mapping' must be a table when "
        "present.");
    return false;
  }

  const auto kAliasMappingPath =
      main_tbl[kKeyAliasMappingPath].value<std::string>();
  if (!kAliasMappingPath.has_value()) {
    modports::EmitError("[Validator] Error: 'alias_mapping_path' must be a string.");
    return false;
  }

  out_paths.alias_mapping_path = *kAliasMappingPath;
  return true;
}

}  // namespace

auto MainRule::Validate(const toml::table& main_tbl, MainConfigPaths& out_paths)
    -> bool {
  return ValidateMainStrictAlias(main_tbl, out_paths);
}

auto MappingRule::Validate(const toml::table& mappings_tbl) -> bool {
  if (!mappings_tbl.contains("text_mappings") ||
      !mappings_tbl["text_mappings"].is_table()) {
    modports::EmitError(
        "[Validator] Error: Mappings config must contain a 'text_mappings' "
        "table.");
    return false;
  }
  return true;
}

auto V2Rule::ValidateAliasMapping(const fs::path& alias_index_path,
                                  const toml::table& alias_tbl) -> bool {
  try {
    static_cast<void>(modalias::LoadAliasMappingDefinition(
        alias_index_path, alias_tbl, modloader::ReadToml));
    return true;
  } catch (const std::exception& error) {
    modports::EmitError("[Validator] Error: alias mapping validation failed: " +
                        std::string(error.what()));
    return false;
  }
}
