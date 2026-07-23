#ifndef INFRASTRUCTURE_CONFIG_LOADER_ALIAS_MAPPING_INDEX_UTILS_HPP_
#define INFRASTRUCTURE_CONFIG_LOADER_ALIAS_MAPPING_INDEX_UTILS_HPP_

#include <toml++/toml.h>

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <functional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

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

inline auto BuildCanonicalValue(std::string_view parent,
                                const std::vector<std::string>& groups,
                                std::string_view leaf_value) -> std::string {
  std::string canonical(parent);
  for (const auto& group : groups) {
    canonical += "_";
    canonical += group;
  }
  canonical += "_";
  canonical += leaf_value;
  return canonical;
}

inline auto BuildCanonicalGroupValue(
    std::string_view parent, const std::vector<std::string>& groups)
    -> std::string {
  std::string canonical(parent);
  for (const auto& group : groups) {
    canonical += "_";
    canonical += group;
  }
  return canonical;
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
  // activity path (the right-hand value).
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
  };
  definition.child_files.reserve(relative_paths.size());

  std::unordered_map<std::string, fs::path> alias_sources;

  const std::function<void(const toml::table&, const AliasMappingChildFile&,
                           std::vector<std::string>&)>
      expand_alias_table =
          [&](const toml::table& aliases_tbl,
              const AliasMappingChildFile& child_file,
              std::vector<std::string>& groups) -> void {
    for (const auto& [alias_key_node, alias_value_node] : aliases_tbl) {
      const std::string alias_key = std::string(alias_key_node.str());
      if (alias_key.empty()) {
        throw std::runtime_error("Alias keys must not be empty in " +
                                 BuildAliasFieldPath(
                                     child_file.relative_path.generic_string(),
                                     groups, ""));
      }

      if (const auto* nested_table = alias_value_node.as_table()) {
        groups.push_back(alias_key);
        expand_alias_table(*nested_table, child_file, groups);
        groups.pop_back();
        continue;
      }

      // A group can be recorded directly as well as contain child aliases.
      // Its aliases resolve to the parent + current group path, while normal
      // string leaves retain the historical parent + groups + leaf form.
      if (alias_key == "group_aliases") {
        if (groups.empty()) {
          throw std::runtime_error(BuildAliasFieldPath(
                                   child_file.relative_path.generic_string(),
                                   groups, alias_key) +
                               " is only valid inside an alias group.");
        }
        const toml::array* group_aliases = alias_value_node.as_array();
        if (group_aliases == nullptr || group_aliases->empty()) {
          throw std::runtime_error(BuildAliasFieldPath(
                                   child_file.relative_path.generic_string(),
                                   groups, alias_key) +
                               " must be a non-empty string array.");
        }
        for (const auto& group_alias_node : *group_aliases) {
          const auto group_alias = group_alias_node.value<std::string>();
          if (!group_alias.has_value() || group_alias->empty()) {
            throw std::runtime_error(BuildAliasFieldPath(
                                     child_file.relative_path.generic_string(),
                                     groups, alias_key) +
                                 " must contain only non-empty strings.");
          }
          const auto [existing_it, inserted] =
              alias_sources.emplace(*group_alias, child_file.relative_path);
          if (!inserted) {
            throw std::runtime_error(
                "Duplicate alias key `" + *group_alias +
                "` across alias child files: " +
                existing_it->second.generic_string() + " and " +
                child_file.relative_path.generic_string());
          }
          definition.expanded_entries.push_back({
              .alias_key = *group_alias,
              .canonical_value =
                  BuildCanonicalGroupValue(child_file.parent, groups),
              .source_path = child_file.relative_path,
          });
        }
        continue;
      }

      const auto leaf_value = alias_value_node.value<std::string>();
      if (!leaf_value.has_value() || leaf_value->empty()) {
        throw std::runtime_error(BuildAliasFieldPath(
                                     child_file.relative_path.generic_string(),
                                     groups, alias_key) +
                                 " must be a non-empty string.");
      }

      const auto [existing_it, inserted] =
          alias_sources.emplace(alias_key, child_file.relative_path);
      if (!inserted) {
        throw std::runtime_error(
            "Duplicate alias key `" + alias_key + "` across alias child files: " +
            existing_it->second.generic_string() + " and " +
            child_file.relative_path.generic_string());
      }

      definition.expanded_entries.push_back({
          .alias_key = alias_key,
          // Canonical paths stay underscore-joined for converter/query flows.
          .canonical_value =
              BuildCanonicalValue(child_file.parent, groups, *leaf_value),
          .source_path = child_file.relative_path,
      });
    }
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
    const auto parent = child_tbl["parent"].value<std::string>();
    if (!parent.has_value() || parent->empty()) {
      throw std::runtime_error("Alias child file must contain a non-empty "
                               "`parent` string: " +
                               relative_path.generic_string());
    }

    const toml::table* aliases_tbl = child_tbl["aliases"].as_table();
    if (aliases_tbl == nullptr) {
      throw std::runtime_error("Alias child file must contain an `aliases` "
                               "table: " +
                               relative_path.generic_string());
    }

    // Read a child file as:
    //   parent -> top-level canonical segment
    //   aliases.<group path> -> middle canonical segments
    //   alias key -> user-authored token
    //   string leaf -> canonical leaf segment
    // `parent` owns the top-level segment; nested alias tables contribute the
    // middle path segments before the string leaf becomes the canonical tail.
    // Child files therefore define the top-level ownership boundary, while the
    // written order of alias entries inside the same file/group remains
    // non-semantic.
    definition.child_files.push_back({
        .relative_path = relative_path,
        .absolute_path = absolute_path,
        .parent = *parent,
    });

    std::vector<std::string> groups;
    expand_alias_table(*aliases_tbl, definition.child_files.back(), groups);
  }

  return definition;
}

}  // namespace tracer::core::infrastructure::config::loader::detail

#endif  // INFRASTRUCTURE_CONFIG_LOADER_ALIAS_MAPPING_INDEX_UTILS_HPP_
