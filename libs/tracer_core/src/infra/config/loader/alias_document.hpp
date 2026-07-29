#ifndef INFRASTRUCTURE_CONFIG_LOADER_ALIAS_DOCUMENT_HPP_
#define INFRASTRUCTURE_CONFIG_LOADER_ALIAS_DOCUMENT_HPP_

#include <toml++/toml.h>

#include <algorithm>
#include <cstddef>
#include <functional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace tracer::core::infrastructure::config::loader::detail {

struct AliasDocumentSourceLocation {
  std::size_t line = 0U;
  std::size_t column = 0U;
};

struct AliasDocumentAlias {
  std::string value;
  AliasDocumentSourceLocation source;
};

enum class AliasDocumentNodeKind {
  kGroup,
  kLeaf,
};

struct AliasDocumentNode {
  AliasDocumentNodeKind kind = AliasDocumentNodeKind::kLeaf;
  std::string canonical_key;
  AliasDocumentSourceLocation source;
  std::vector<AliasDocumentAlias> aliases;
  std::vector<AliasDocumentNode> children;
};

struct AliasDocument {
  std::string parent;
  AliasDocumentSourceLocation parent_source;
  std::vector<AliasDocumentNode> nodes;
};

struct AliasDocumentCanonicalNode {
  std::string canonical;
  std::vector<std::string> path;
  AliasDocumentNodeKind kind = AliasDocumentNodeKind::kLeaf;
  const AliasDocumentNode* node = nullptr;
};

class AliasDocumentParseError final : public std::runtime_error {
 public:
  AliasDocumentParseError(AliasDocumentSourceLocation source,
                          std::vector<std::string> groups,
                          std::string field, std::string message)
      : std::runtime_error(std::move(message)),
        source_(source),
        groups_(std::move(groups)),
        field_(std::move(field)) {}

  [[nodiscard]] auto source() const -> AliasDocumentSourceLocation {
    return source_;
  }

  [[nodiscard]] auto groups() const -> const std::vector<std::string>& {
    return groups_;
  }

  [[nodiscard]] auto field() const -> std::string_view { return field_; }

 private:
  AliasDocumentSourceLocation source_;
  std::vector<std::string> groups_;
  std::string field_;
};

inline auto AliasDocumentSource(const toml::source_region& source)
    -> AliasDocumentSourceLocation {
  return {
      .line = static_cast<std::size_t>(source.begin.line),
      .column = static_cast<std::size_t>(source.begin.column),
  };
}

inline auto BuildAliasCanonicalPath(std::string_view parent,
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

inline auto ParseAliasDocument(const toml::table& table) -> AliasDocument {
  const toml::node* parent_node = table.get("parent");
  const auto parent = parent_node == nullptr
                          ? std::optional<std::string>{}
                          : parent_node->value<std::string>();
  if (!parent.has_value() || parent->empty()) {
    throw AliasDocumentParseError(
        parent_node == nullptr ? AliasDocumentSourceLocation{1U, 1U}
                               : AliasDocumentSource(parent_node->source()),
        {}, "parent", "Alias TOML must contain a non-empty `parent` string.");
  }

  const toml::node* aliases_node = table.get("aliases");
  const toml::table* aliases =
      aliases_node == nullptr ? nullptr : aliases_node->as_table();
  if (aliases == nullptr) {
    throw AliasDocumentParseError(
        aliases_node == nullptr ? AliasDocumentSourceLocation{1U, 1U}
                                : AliasDocumentSource(aliases_node->source()),
                                {}, "aliases", "Alias TOML must contain an `aliases` table.");
  }

  const std::function<std::vector<AliasDocumentAlias>(
      const toml::node&, const std::vector<std::string>&, std::string_view)>
      parse_alias_array =
          [](const toml::node& node, const std::vector<std::string>& groups,
             std::string_view field) -> std::vector<AliasDocumentAlias> {
    const toml::array* values = node.as_array();
    if (values == nullptr || values->empty()) {
      throw AliasDocumentParseError(
          AliasDocumentSource(node.source()), groups, std::string(field),
          "Alias field must be a non-empty string array.");
    }
    std::vector<AliasDocumentAlias> aliases;
    aliases.reserve(values->size());
    for (const auto& item : *values) {
      const auto value = item.value<std::string>();
      if (!value.has_value() || value->empty()) {
        throw AliasDocumentParseError(
            AliasDocumentSource(item.source()), groups, std::string(field),
            "Alias field must contain only non-empty strings.");
      }
      aliases.push_back({
          .value = *value,
          .source = AliasDocumentSource(item.source()),
      });
    }
    return aliases;
  };

  std::function<std::vector<AliasDocumentNode>(
      const toml::table&, const std::vector<std::string>&, bool)>
      parse_nodes;
  parse_nodes = [&](const toml::table& current,
                    const std::vector<std::string>& groups,
                    bool is_group) -> std::vector<AliasDocumentNode> {
    std::vector<AliasDocumentNode> nodes;
    for (const auto& [key_node, value_node] : current) {
      const std::string key(key_node.str());
      const AliasDocumentSourceLocation source =
          AliasDocumentSource(key_node.source());
      if (key.empty()) {
        throw AliasDocumentParseError(
            source, groups, {}, "Alias canonical keys must not be empty.");
      }
      if (key == "group_aliases") {
        if (!is_group) {
          throw AliasDocumentParseError(
              AliasDocumentSource(value_node.source()), groups, key,
              "`group_aliases` is only valid inside an alias group.");
        }
        continue;
      }

      if (const toml::table* child = value_node.as_table()) {
        auto child_groups = groups;
        child_groups.push_back(key);
        const toml::node* group_aliases = child->get("group_aliases");
        AliasDocumentNode group{
            .kind = AliasDocumentNodeKind::kGroup,
            .canonical_key = key,
            .source = source,
            .aliases = group_aliases == nullptr
                           ? std::vector<AliasDocumentAlias>{}
                           : parse_alias_array(*group_aliases, child_groups,
                                               "group_aliases"),
            .children = parse_nodes(*child, child_groups, true),
        };
        nodes.push_back(std::move(group));
        continue;
      }

      nodes.push_back({
          .kind = AliasDocumentNodeKind::kLeaf,
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
      .parent_source = AliasDocumentSource(parent_node->source()),
      .nodes = parse_nodes(*aliases, {}, false),
  };
}

inline auto CollectAliasDocumentCanonicalNodes(const AliasDocument& document)
    -> std::vector<AliasDocumentCanonicalNode> {
  std::vector<AliasDocumentCanonicalNode> result;
  const std::function<void(const std::vector<AliasDocumentNode>&,
                           std::vector<std::string>&)>
      collect = [&](const std::vector<AliasDocumentNode>& nodes,
                    std::vector<std::string>& path) -> void {
    for (const auto& node : nodes) {
      path.push_back(node.canonical_key);
      result.push_back({
          .canonical = BuildAliasCanonicalPath(
              document.parent,
              node.kind == AliasDocumentNodeKind::kGroup
                  ? path
                  : std::vector<std::string>(path.begin(), path.end() - 1),
              node.kind == AliasDocumentNodeKind::kGroup ? std::string_view{}
                                                         : node.canonical_key),
          .path = path,
          .kind = node.kind,
          .node = &node,
      });
      if (node.kind == AliasDocumentNodeKind::kGroup) {
        collect(node.children, path);
      }
      path.pop_back();
    }
  };

  std::vector<std::string> path;
  collect(document.nodes, path);
  return result;
}

inline auto ValidateAliasDocumentAliasUniqueness(
    const AliasDocument& document) -> void {
  std::vector<AliasDocumentAlias> seen;
  for (const auto& canonical_node :
       CollectAliasDocumentCanonicalNodes(document)) {
    for (const auto& alias : canonical_node.node->aliases) {
      const auto existing = std::ranges::find_if(
          seen, [&alias](const AliasDocumentAlias& candidate) {
            return candidate.value == alias.value;
          });
      if (existing != seen.end()) {
        throw AliasDocumentParseError(
            alias.source, {}, alias.value,
            "Duplicate alias key `" + alias.value + "`.");
      }
      seen.push_back(alias);
    }
  }
}

}  // namespace tracer::core::infrastructure::config::loader::detail

#endif  // INFRASTRUCTURE_CONFIG_LOADER_ALIAS_DOCUMENT_HPP_
