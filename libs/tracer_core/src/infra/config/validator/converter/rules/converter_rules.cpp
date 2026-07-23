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

auto ValidateMainStrictAlias(const toml::table& main_tbl) -> bool {
  const std::set<std::string> kRequiredKeys = {"sleep_inference"};

  for (const auto& key : kRequiredKeys) {
    if (!main_tbl.contains(key)) {
      modports::EmitError(
          "[Validator] Error: Main config is missing required key: '" + key +
          "'");
      return false;
    }
  }

  const toml::table* sleep_inference_tbl =
      main_tbl["sleep_inference"].as_table();
  if (sleep_inference_tbl == nullptr ||
      !sleep_inference_tbl->get_as<toml::array>("wake_keywords") ||
      !sleep_inference_tbl->get("sleep_project_path") ||
      !sleep_inference_tbl->get("sleep_project_path")
           ->value<std::string>()
           .has_value()) {
    modports::EmitError(
        "[Validator] Error: 'sleep_inference' must contain 'wake_keywords' and a string "
        "'sleep_project_path'.");
    return false;
  }
  if (main_tbl.contains("top_parent_mapping") &&
      !main_tbl["top_parent_mapping"].is_table()) {
    modports::EmitError(
        "[Validator] Error: 'top_parent_mapping' must be a table when "
        "present.");
    return false;
  }

  return true;
}

}  // namespace

auto MainRule::Validate(const toml::table& main_tbl) -> bool {
  return ValidateMainStrictAlias(main_tbl);
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

auto V2Rule::ValidateAliasMapping(const fs::path& alias_directory) -> bool {
  try {
    static_cast<void>(modalias::LoadAliasMappingDefinition(alias_directory,
                                                           modloader::ReadToml));
    return true;
  } catch (const std::exception& error) {
    modports::EmitError("[Validator] Error: alias mapping validation failed: " +
                        std::string(error.what()));
    return false;
  }
}
