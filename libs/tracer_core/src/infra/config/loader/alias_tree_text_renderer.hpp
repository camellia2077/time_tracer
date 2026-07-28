#ifndef INFRASTRUCTURE_CONFIG_LOADER_ALIAS_TREE_TEXT_RENDERER_HPP_
#define INFRASTRUCTURE_CONFIG_LOADER_ALIAS_TREE_TEXT_RENDERER_HPP_

#include <algorithm>
#include <filesystem>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "infra/config/loader/alias_document.hpp"

namespace tracer::core::infrastructure::config::loader::detail {

namespace fs = std::filesystem;

struct AliasTreeTextNode {
  std::string name;
  AliasDocumentNodeKind kind = AliasDocumentNodeKind::kLeaf;
  std::vector<std::string> aliases;
  std::vector<AliasTreeTextNode> children;
};

inline auto BuildAliasTreeTextNode(const AliasDocumentNode& document_node)
    -> AliasTreeTextNode {
  AliasTreeTextNode node{
      .name = document_node.canonical_key,
      .kind = document_node.kind,
      .aliases = {},
      .children = {},
  };
  node.aliases.reserve(document_node.aliases.size());
  for (const auto& alias : document_node.aliases) {
    node.aliases.push_back(alias.value);
  }
  std::ranges::sort(node.aliases);

  node.children.reserve(document_node.children.size());
  for (const auto& child : document_node.children) {
    node.children.push_back(BuildAliasTreeTextNode(child));
  }
  std::ranges::sort(node.children,
                    [](const AliasTreeTextNode& left,
                       const AliasTreeTextNode& right) {
                      return left.name < right.name;
                    });
  return node;
}

inline auto RenderAliasTreeNode(const AliasTreeTextNode& node,
                                std::string prefix, bool is_last,
                                bool is_root, bool show_aliases,
                                std::ostringstream& output) -> void {
  if (is_root) {
    output << node.name;
  } else {
    output << prefix << (is_last ? "└── " : "├── ") << node.name;
  }

  if (show_aliases && !node.aliases.empty()) {
    output << " — "
           << (node.kind == AliasDocumentNodeKind::kGroup ? "group_aliases"
                                                          : "aliases")
           << ": ";
    for (std::size_t index = 0U; index < node.aliases.size(); ++index) {
      if (index != 0U) {
        output << ", ";
      }
      output << node.aliases[index];
    }
  }
  output << '\n';

  const std::string child_prefix =
      is_root ? std::string() : prefix + (is_last ? "    " : "│   ");
  for (std::size_t index = 0U; index < node.children.size(); ++index) {
    RenderAliasTreeNode(node.children[index], child_prefix,
                        index + 1U == node.children.size(), false,
                        show_aliases, output);
  }
}

inline auto RenderAliasTreeText(const AliasDocument& document,
                                bool show_aliases) -> std::string {
  AliasTreeTextNode root{
      .name = document.parent,
      .kind = AliasDocumentNodeKind::kGroup,
      .aliases = {},
      .children = {},
  };
  root.children.reserve(document.nodes.size());
  for (const auto& node : document.nodes) {
    root.children.push_back(BuildAliasTreeTextNode(node));
  }
  std::ranges::sort(root.children,
                    [](const AliasTreeTextNode& left,
                       const AliasTreeTextNode& right) {
                      return left.name < right.name;
                    });

  std::ostringstream output;
  RenderAliasTreeNode(root, {}, true, true, show_aliases, output);
  return output.str();
}

inline auto RenderAliasTreeText(const fs::path& alias_toml_path,
                                bool show_aliases) -> std::string {
  const fs::path path = fs::absolute(alias_toml_path);
  if (!fs::exists(path) || !fs::is_regular_file(path)) {
    throw std::runtime_error("Alias TOML file not found: " + path.string());
  }
  return RenderAliasTreeText(ParseAliasDocument(toml::parse_file(path.string())),
                             show_aliases);
}

}  // namespace tracer::core::infrastructure::config::loader::detail

#endif  // INFRASTRUCTURE_CONFIG_LOADER_ALIAS_TREE_TEXT_RENDERER_HPP_
