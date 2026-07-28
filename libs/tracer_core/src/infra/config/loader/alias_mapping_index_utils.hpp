#ifndef INFRASTRUCTURE_CONFIG_LOADER_ALIAS_MAPPING_INDEX_UTILS_HPP_
#define INFRASTRUCTURE_CONFIG_LOADER_ALIAS_MAPPING_INDEX_UTILS_HPP_

#include <toml++/toml.h>

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <functional>
#include <fstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

#include "infra/config/loader/alias_document.hpp"

namespace tracer::core::infrastructure::config::loader::detail {

namespace fs = std::filesystem;

struct AliasMappingChildFile {
  fs::path relative_path;
  fs::path absolute_path;
  std::string parent;
};

struct ExpandedAliasMappingEntry {
  std::string alias_key;
  std::string canonical_value;
  fs::path source_path;
};

struct AliasMappingDefinition {
  fs::path alias_directory_path;
  std::vector<AliasMappingChildFile> child_files;
  std::vector<ExpandedAliasMappingEntry> expanded_entries;
};

struct AliasSourceLocation {
  fs::path path;
  std::size_t line = 0U;
  std::size_t column = 0U;
};

inline auto NormalizeRelativeTomlPath(std::string path_text) -> fs::path {
  std::replace(path_text.begin(), path_text.end(), '\\', '/');
  return fs::path(path_text).lexically_normal();
}

inline auto IsTomlPath(const fs::path& path) -> bool {
  std::string extension = path.extension().string();
  std::transform(extension.begin(), extension.end(), extension.begin(),
                 [](unsigned char value) {
                   return static_cast<char>(std::tolower(value));
                 });
  return extension == ".toml";
}

inline auto IsAliasChildTomlPath(const fs::path& path) -> bool {
  return IsTomlPath(path) && path.filename() != "_system.toml";
}

inline auto BuildAliasFieldPath(std::string_view relative_child_path,
                                const std::vector<std::string>& groups,
                                std::string_view leaf_key) -> std::string {
  std::string field = "alias child file `" + std::string(relative_child_path) +
                      "` field `aliases";
  for (const auto& group : groups) {
    field += ".";
    field += group;
  }
  if (!leaf_key.empty()) {
    field += ".";
    field += leaf_key;
  }
  field += "`";
  return field;
}

inline auto ReadTomlSourceLine(const fs::path& path, std::size_t line_number)
    -> std::string {
  if (line_number == 0U) {
    return {};
  }
  std::ifstream input(path, std::ios::binary);
  if (!input.is_open()) {
    return {};
  }
  std::string line;
  for (std::size_t current = 1U; current <= line_number; ++current) {
    if (!std::getline(input, line)) {
      return {};
    }
  }
  return line;
}

inline auto BuildTomlDiagnostic(const fs::path& path, std::size_t line_number,
                                std::size_t column_number,
                                std::string_view message) -> std::string {
  std::string diagnostic = path.string();
  if (line_number > 0U) {
    diagnostic += ":" + std::to_string(line_number);
    if (column_number > 0U) {
      diagnostic += ":" + std::to_string(column_number);
    }
  }
  diagnostic += ": ";
  diagnostic += message;
  const std::string raw_line = ReadTomlSourceLine(path, line_number);
  if (!raw_line.empty()) {
    diagnostic += "\n> ";
    diagnostic += raw_line;
  }
  return diagnostic;
}

inline auto BuildTomlDiagnostic(const fs::path& path,
                                const toml::source_region& source,
                                std::string_view message) -> std::string {
  return BuildTomlDiagnostic(
      path, static_cast<std::size_t>(source.begin.line),
      static_cast<std::size_t>(source.begin.column), message);
}

inline auto BuildTomlDiagnostic(const fs::path& path, const toml::node& node,
                                std::string_view message) -> std::string {
  return BuildTomlDiagnostic(path, node.source(), message);
}

inline auto BuildTomlDiagnostic(const fs::path& path,
                                AliasDocumentSourceLocation source,
                                std::string_view message) -> std::string {
  return BuildTomlDiagnostic(path, source.line, source.column, message);
}

inline auto BuildAliasChildParseHint(const fs::path& relative_path,
                                     std::string_view error_message)
    -> std::string {
  std::string message(error_message);
  if (message.find("Config TOML Parse Error [") == std::string::npos) {
    return message;
  }

  return message +
         " | Alias child files are encoded as TOML table paths such as "
         "`[aliases.study.math]`. In TOML table headers, unquoted path "
         "segments cannot contain spaces. This is a TOML syntax requirement, "
         "not an alias-timing or database rule. If a canonical path segment "
         "currently contains spaces, rewrite that segment with a TOML-safe "
         "form such as `data-structure` before converting it into child-file "
         "table headers. Source child file: " +
         relative_path.generic_string();
}

template <typename ReadTomlFunc>
inline auto LoadAliasMappingDefinition(const fs::path& alias_directory_path,
                                       ReadTomlFunc&& read_toml)
    -> AliasMappingDefinition {
  // Alias mapping only normalizes user-authored activity-name tokens.
  // Its responsibility is limited to resolving an alias key into a canonical
  // activity path. Alias child files use the strict canonical-keyed shape:
  // each ordinary entry has a canonical leaf key and a non-empty string array
  // of aliases.
  //
  // Design rules:
  // 1. Every alias key must be globally unambiguous.
  //    A given alias key must always resolve to exactly one canonical activity path.
  // 2. Canonical activity paths do not need to be unique.
  //    Different alias keys may resolve to the same canonical activity path.
  // 3. Duplicate alias keys are rejected strictly, even if the right-hand value
  //    is identical, because repeated declarations are treated as redundant
  //    configuration.
  //
  // Non-goals:
  // This alias mapping layer does not define or carry any timing semantics.
  // It is not responsible for time points, start/end times, durations, how many
  // time ranges may reference the same activity, or how activity records are
  // later inserted into or queried from the database.
  // Those concerns belong to later conversion, persistence, and query stages.
  //
  const fs::path normalized_alias_directory = fs::absolute(alias_directory_path);
  if (!fs::exists(normalized_alias_directory) ||
      !fs::is_directory(normalized_alias_directory)) {
    throw std::runtime_error("Alias directory not found: " +
                             normalized_alias_directory.string());
  }

  std::vector<fs::path> relative_paths;
  for (const auto& entry : fs::recursive_directory_iterator(
           normalized_alias_directory)) {
    if (entry.is_regular_file() && IsAliasChildTomlPath(entry.path())) {
      relative_paths.push_back(
          fs::relative(entry.path(), normalized_alias_directory));
    }
  }
  std::ranges::sort(relative_paths, [](const fs::path& left, const fs::path& right) {
    return left.generic_string() < right.generic_string();
  });
  if (relative_paths.empty()) {
    throw std::runtime_error("Alias directory contains no TOML files: " +
                             normalized_alias_directory.string());
  }

  AliasMappingDefinition definition{
      .alias_directory_path = normalized_alias_directory,
      .child_files = {},
      .expanded_entries = {},
  };
  definition.child_files.reserve(relative_paths.size());

  std::unordered_map<std::string, AliasSourceLocation> alias_sources;
  const auto add_expanded_alias =
      [&](const AliasMappingChildFile& child_file,
          const AliasDocumentAlias& alias, std::string_view canonical) -> void {
    const auto [existing_it, inserted] = alias_sources.emplace(
        alias.value, AliasSourceLocation{child_file.absolute_path,
                                         alias.source.line,
                                         alias.source.column});
    if (!inserted) {
      throw std::runtime_error(BuildTomlDiagnostic(
          child_file.absolute_path, alias.source,
          "Duplicate alias key `" + alias.value + "`; first defined at " +
              BuildTomlDiagnostic(existing_it->second.path,
                                  existing_it->second.line,
                                  existing_it->second.column, "here")));
    }
    definition.expanded_entries.push_back({
        .alias_key = alias.value,
        .canonical_value = std::string(canonical),
        .source_path = child_file.relative_path,
    });
  };

  for (const fs::path& relative_path : relative_paths) {
    const fs::path absolute_path = normalized_alias_directory / relative_path;
    if (!fs::exists(absolute_path) || !fs::is_regular_file(absolute_path)) {
      throw std::runtime_error("Alias child file not found: " +
                               absolute_path.string());
    }

    toml::table child_tbl;
    try {
      child_tbl = read_toml(absolute_path);
    } catch (const std::exception& error) {
      throw std::runtime_error(
          BuildAliasChildParseHint(relative_path, error.what()));
    }
    AliasDocument document;
    try {
      document = ParseAliasDocument(child_tbl);
    } catch (const AliasDocumentParseError& error) {
      throw std::runtime_error(
          BuildTomlDiagnostic(absolute_path, error.source(), error.what()));
    }

    // Read a child file as:
    //   parent -> top-level canonical segment
    //   aliases.<group path> -> middle canonical segments
    //   canonical leaf key -> user-authored alias string array
    // `parent` owns the top-level segment; nested alias tables contribute the
    // middle path segments before the string leaf becomes the canonical tail.
    // Child files therefore define the top-level ownership boundary, while the
    // written order of alias entries inside the same file/group remains
    // non-semantic.
    definition.child_files.push_back({
        .relative_path = relative_path,
        .absolute_path = absolute_path,
        .parent = document.parent,
    });

    for (const auto& canonical_node :
         CollectAliasDocumentCanonicalNodes(document)) {
      for (const auto& alias : canonical_node.node->aliases) {
        add_expanded_alias(definition.child_files.back(), alias,
                           canonical_node.canonical);
      }
    }
  }

  return definition;
}

}  // namespace tracer::core::infrastructure::config::loader::detail

#endif  // INFRASTRUCTURE_CONFIG_LOADER_ALIAS_MAPPING_INDEX_UTILS_HPP_
