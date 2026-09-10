#ifndef INFRASTRUCTURE_CONFIG_LOADER_BEHAVIOR_CANONICAL_UTILS_HPP_
#define INFRASTRUCTURE_CONFIG_LOADER_BEHAVIOR_CANONICAL_UTILS_HPP_

#include <toml++/toml.h>

#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace tracer::core::infrastructure::config::loader::detail {

// behavior.toml uses the canonical tree for two related projections:
// activity-name aliases and wake aliases. Keep the traversal and validation in
// one place so the module and non-module loader implementations cannot drift.
inline auto PopulateBehaviorCanonicalMappings(
    const toml::table& canonical, std::string_view parent,
    toml::table& text_mappings, std::vector<std::string>& wake_keywords)
    -> void {
  const auto visit = [&](const auto& self, const toml::table& table,
                        const std::string& path) -> void {
    for (const auto& [key_node, value_node] : table) {
      const std::string key(key_node.str());
      if (key == "group_aliases") {
        continue;
      }

      if (const auto* child = value_node.as_table()) {
        const std::string child_path = path.empty() ? key : path + "_" + key;
        self(self, *child, child_path);
        continue;
      }

      const auto* aliases = value_node.as_array();
      if (aliases == nullptr) {
        throw std::runtime_error(
            "Behavior canonical entries must be string arrays.");
      }
      if (aliases->empty()) {
        throw std::runtime_error(
            "Behavior canonical aliases must be a non-empty string array.");
      }

      const std::string canonical_name =
          std::string(parent) + "_" + (path.empty() ? key : path + "_" + key);
      for (const auto& alias_node : *aliases) {
        const auto alias = alias_node.value<std::string>();
        if (!alias.has_value() || alias->empty()) {
          throw std::runtime_error(
              "Behavior canonical aliases must be non-empty strings.");
        }
        if (text_mappings.contains(*alias)) {
          throw std::runtime_error(
              "Duplicate alias key in behavior.toml: " + *alias);
        }
        text_mappings.insert(*alias, canonical_name);
        wake_keywords.push_back(*alias);
      }
    }
  };

  visit(visit, canonical, {});
  if (wake_keywords.empty()) {
    throw std::runtime_error(
        "Behavior canonical table must define at least one alias.");
  }
}

}  // namespace tracer::core::infrastructure::config::loader::detail

#endif  // INFRASTRUCTURE_CONFIG_LOADER_BEHAVIOR_CANONICAL_UTILS_HPP_
