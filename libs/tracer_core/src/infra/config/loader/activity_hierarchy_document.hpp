#ifndef INFRASTRUCTURE_CONFIG_LOADER_ACTIVITY_HIERARCHY_DOCUMENT_HPP_
#define INFRASTRUCTURE_CONFIG_LOADER_ACTIVITY_HIERARCHY_DOCUMENT_HPP_

#include <toml++/toml.h>

#include <algorithm>
#include <cctype>
#include <cstddef>
#include <functional>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace tracer::core::infrastructure::config::loader::detail {

struct ActivityHierarchyDocumentSourceLocation {
  std::size_t line = 0U;
  std::size_t column = 0U;
};

struct ActivityHierarchyAlias {
  std::string value;
  ActivityHierarchyDocumentSourceLocation source;
};

enum class ActivityHierarchyDocumentNodeKind {
  kGroup,
  kLeaf,
};

struct ActivityHierarchyNode {
  ActivityHierarchyDocumentNodeKind kind = ActivityHierarchyDocumentNodeKind::kLeaf;
  std::string canonical_key;
  ActivityHierarchyDocumentSourceLocation source;
  std::vector<ActivityHierarchyAlias> aliases;
  std::vector<ActivityHierarchyNode> children;
};

struct ActivityHierarchyDocument {
  std::string parent;
  ActivityHierarchyDocumentSourceLocation parent_source;
  // A parent-level visual identifier for presentation consumers. It does not
  // participate in canonical-path or alias resolution semantics.
  std::optional<std::string> color;
  std::vector<ActivityHierarchyNode> nodes;
};

struct ActivityHierarchyCanonicalNode {
  std::string canonical;
  std::vector<std::string> path;
  ActivityHierarchyDocumentNodeKind kind = ActivityHierarchyDocumentNodeKind::kLeaf;
  const ActivityHierarchyNode* node = nullptr;
};

class ActivityHierarchyDocumentParseError final : public std::runtime_error {
 public:
  ActivityHierarchyDocumentParseError(ActivityHierarchyDocumentSourceLocation source,
                          std::vector<std::string> groups, std::string field,
                          std::string message)
      : std::runtime_error(std::move(message)),
        source_(source),
        groups_(std::move(groups)),
        field_(std::move(field)) {}

  [[nodiscard]] auto source() const -> ActivityHierarchyDocumentSourceLocation {
    return source_;
  }

  [[nodiscard]] auto groups() const -> const std::vector<std::string>& {
    return groups_;
  }

  [[nodiscard]] auto field() const -> std::string_view { return field_; }

 private:
  ActivityHierarchyDocumentSourceLocation source_;
  std::vector<std::string> groups_;
  std::string field_;
};

inline auto ActivityHierarchyDocumentSource(const toml::source_region& source)
    -> ActivityHierarchyDocumentSourceLocation {
  return {
      .line = static_cast<std::size_t>(source.begin.line),
      .column = static_cast<std::size_t>(source.begin.column),
  };
}

inline auto BuildActivityHierarchyCanonicalPath(std::string_view parent,
                                    const std::vector<std::string>& groups,
                                    std::string_view leaf = {}) -> std::string {
  std::string canonical(parent);
  for (const auto& group : groups) {
    canonical += "_";
    canonical += group;
  }
  if (!leaf.empty()) {
    canonical += "_";
    canonical += leaf;
  }
  return canonical;
}

inline auto IsActivityHierarchyParentColor(std::string_view color) -> bool {
  return color.size() == 7U && color.front() == '#' &&
         std::ranges::all_of(color.substr(1), [](unsigned char value) {
           return std::isxdigit(value) != 0;
         });
}

inline auto ParseActivityHierarchyDocument(const toml::table& table) -> ActivityHierarchyDocument {
  for (const auto& [key_node, value_node] : table) {
    const std::string key(key_node.str());
    if (key != "parent" && key != "color" && key != "canonical") {
      throw ActivityHierarchyDocumentParseError(
          ActivityHierarchyDocumentSource(key_node.source()), {}, key,
          "Canonical TOML contains unsupported top-level field `" + key + "`.");
    }
  }

  const toml::node* parent_node = table.get("parent");
  const auto parent = parent_node == nullptr
                          ? std::optional<std::string>{}
                          : parent_node->value<std::string>();
  if (!parent.has_value() || parent->empty()) {
    throw ActivityHierarchyDocumentParseError(
        parent_node == nullptr ? ActivityHierarchyDocumentSourceLocation{1U, 1U}
                               : ActivityHierarchyDocumentSource(parent_node->source()),
        {}, "parent",
        "Canonical TOML must contain a non-empty `parent` string.");
  }

  const toml::node* color_node = table.get("color");
  std::optional<std::string> color;
  if (color_node != nullptr) {
    color = color_node->value<std::string>();
    if (!color.has_value() || !IsActivityHierarchyParentColor(*color)) {
      throw ActivityHierarchyDocumentParseError(
          ActivityHierarchyDocumentSource(color_node->source()), {}, "color",
          "Optional `color` must be an uppercase or lowercase #RRGGBB string.");
    }
  }

  const toml::node* canonical_node = table.get("canonical");
  const toml::table* canonical =
      canonical_node == nullptr ? nullptr : canonical_node->as_table();
  if (canonical == nullptr) {
    throw ActivityHierarchyDocumentParseError(
        canonical_node == nullptr
            ? ActivityHierarchyDocumentSourceLocation{1U, 1U}
            : ActivityHierarchyDocumentSource(canonical_node->source()),
        {}, "canonical", "Canonical TOML must contain a `canonical` table.");
  }

  const std::function<std::vector<ActivityHierarchyAlias>(
      const toml::node&, const std::vector<std::string>&, std::string_view)>
      parse_alias_array =
          [](const toml::node& node, const std::vector<std::string>& groups,
             std::string_view field) -> std::vector<ActivityHierarchyAlias> {
    const toml::array* values = node.as_array();
    if (values == nullptr || values->empty()) {
      throw ActivityHierarchyDocumentParseError(
          ActivityHierarchyDocumentSource(node.source()), groups, std::string(field),
          "Alias field must be a non-empty string array.");
    }
    std::vector<ActivityHierarchyAlias> aliases;
    aliases.reserve(values->size());
    for (const auto& item : *values) {
      const auto value = item.value<std::string>();
      if (!value.has_value() || value->empty()) {
        throw ActivityHierarchyDocumentParseError(
            ActivityHierarchyDocumentSource(item.source()), groups, std::string(field),
            "Alias field must contain only non-empty strings.");
      }
      aliases.push_back({
          .value = *value,
          .source = ActivityHierarchyDocumentSource(item.source()),
      });
    }
    return aliases;
  };

  std::function<std::vector<ActivityHierarchyNode>(
      const toml::table&, const std::vector<std::string>&, bool)>
      parse_nodes;
  parse_nodes = [&](const toml::table& current,
                    const std::vector<std::string>& groups,
                    bool is_group) -> std::vector<ActivityHierarchyNode> {
    std::vector<ActivityHierarchyNode> nodes;
    for (const auto& [key_node, value_node] : current) {
      const std::string key(key_node.str());
      const ActivityHierarchyDocumentSourceLocation source =
          ActivityHierarchyDocumentSource(key_node.source());
      if (key.empty()) {
        throw ActivityHierarchyDocumentParseError(
            source, groups, {}, "Alias canonical keys must not be empty.");
      }
      if (key == "group_aliases") {
        if (!is_group) {
          throw ActivityHierarchyDocumentParseError(
              ActivityHierarchyDocumentSource(value_node.source()), groups, key,
              "`group_aliases` is only valid inside an alias group.");
        }
        continue;
      }

      if (const toml::table* child = value_node.as_table()) {
        auto child_groups = groups;
        child_groups.push_back(key);
        const toml::node* group_aliases = child->get("group_aliases");
        ActivityHierarchyNode group{
            .kind = ActivityHierarchyDocumentNodeKind::kGroup,
            .canonical_key = key,
            .source = source,
            .aliases = group_aliases == nullptr
                           ? std::vector<ActivityHierarchyAlias>{}
                           : parse_alias_array(*group_aliases, child_groups,
                                               "group_aliases"),
            .children = parse_nodes(*child, child_groups, true),
        };
        nodes.push_back(std::move(group));
        continue;
      }

      nodes.push_back({
          .kind = ActivityHierarchyDocumentNodeKind::kLeaf,
          .canonical_key = key,
          .source = source,
          .aliases = parse_alias_array(value_node, groups, key),
          .children = {},
      });
    }
    return nodes;
  };

  return {
      .parent = *parent,
      .parent_source = ActivityHierarchyDocumentSource(parent_node->source()),
      .color = std::move(color),
      .nodes = parse_nodes(*canonical, {}, false),
  };
}

inline auto CollectActivityHierarchyCanonicalNodes(const ActivityHierarchyDocument& document)
    -> std::vector<ActivityHierarchyCanonicalNode> {
  std::vector<ActivityHierarchyCanonicalNode> result;
  const std::function<void(const std::vector<ActivityHierarchyNode>&,
                           std::vector<std::string>&)>
      collect = [&](const std::vector<ActivityHierarchyNode>& nodes,
                    std::vector<std::string>& path) -> void {
    for (const auto& node : nodes) {
      path.push_back(node.canonical_key);
      result.push_back({
          .canonical = BuildActivityHierarchyCanonicalPath(
              document.parent,
              node.kind == ActivityHierarchyDocumentNodeKind::kGroup
                  ? path
                  : std::vector<std::string>(path.begin(), path.end() - 1),
              node.kind == ActivityHierarchyDocumentNodeKind::kGroup ? std::string_view{}
                                                         : node.canonical_key),
          .path = path,
          .kind = node.kind,
          .node = &node,
      });
      if (node.kind == ActivityHierarchyDocumentNodeKind::kGroup) {
        collect(node.children, path);
      }
      path.pop_back();
    }
  };

  std::vector<std::string> path;
  collect(document.nodes, path);
  return result;
}

inline auto ValidateActivityHierarchyAliasUniqueness(const ActivityHierarchyDocument& document)
    -> void {
  std::vector<ActivityHierarchyAlias> seen;
  for (const auto& canonical_node :
       CollectActivityHierarchyCanonicalNodes(document)) {
    for (const auto& alias : canonical_node.node->aliases) {
      const auto existing = std::ranges::find_if(
          seen, [&alias](const ActivityHierarchyAlias& candidate) {
            return candidate.value == alias.value;
          });
      if (existing != seen.end()) {
        throw ActivityHierarchyDocumentParseError(
            alias.source, {}, alias.value,
            "Duplicate alias key `" + alias.value + "`.");
      }
      seen.push_back(alias);
    }
  }
}

}  // namespace tracer::core::infrastructure::config::loader::detail

#endif  // INFRASTRUCTURE_CONFIG_LOADER_ACTIVITY_HIERARCHY_DOCUMENT_HPP_
