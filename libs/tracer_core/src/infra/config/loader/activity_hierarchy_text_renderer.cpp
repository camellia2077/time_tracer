#include "application/ports/config/activity_hierarchy_text_renderer.hpp"

#include <algorithm>
#include <fstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "application/ports/config/alias_toml_editor.hpp"
#include "infra/config/loader/activity_hierarchy_text_renderer.hpp"

namespace tracer::core::application::config {

auto RenderActivityHierarchyText(
    const std::filesystem::path& activity_hierarchy_toml_path,
    bool show_aliases) -> std::string {
  const auto path = std::filesystem::absolute(activity_hierarchy_toml_path);
  if (!std::filesystem::exists(path) ||
      !std::filesystem::is_regular_file(path)) {
    throw std::runtime_error("Alias TOML file not found: " + path.string());
  }
  std::ifstream input(path);
  if (!input) {
    throw std::runtime_error("Read alias TOML file failed: " + path.string());
  }
  const std::string content((std::istreambuf_iterator<char>(input)),
                            std::istreambuf_iterator<char>());
  return RenderActivityHierarchyText(std::string_view(content), show_aliases);
}

auto RenderActivityHierarchyText(std::string_view toml_content, bool show_aliases)
    -> std::string {
  return infrastructure::config::loader::detail::RenderActivityHierarchyTreeText(
      DescribeActivityHierarchy(toml_content), show_aliases);
}

}  // namespace tracer::core::application::config

namespace tracer::core::infrastructure::config::loader::detail {
namespace {

namespace config = tracer::core::application::config;

auto RenderNode(const config::ActivityHierarchyTreeNode& node, std::string prefix,
                bool is_last, bool show_aliases, std::string& output) -> void {
  output += prefix;
  output += is_last ? "└── " : "├── ";
  output += node.canonical_key;
  if (show_aliases && !node.aliases.empty()) {
    output += " — ";
    output += node.IsGroup() ? "group_aliases: " : "aliases: ";
    auto aliases = node.aliases;
    std::ranges::sort(aliases);
    for (std::size_t index = 0U; index < aliases.size(); ++index) {
      if (index != 0U) {
        output += ", ";
      }
      output += aliases[index];
    }
  }
  output += '\n';

  const std::string child_prefix =
      prefix + (is_last ? "    " : "│   ");
  auto children = node.children;
  std::ranges::sort(
      children, [](const auto& left, const auto& right) {
        return left.canonical_key < right.canonical_key;
      });
  for (std::size_t index = 0U; index < children.size(); ++index) {
    RenderNode(children[index], child_prefix, index + 1U == children.size(),
               show_aliases, output);
  }
}

}  // namespace

auto RenderActivityHierarchyTreeText(const config::ActivityHierarchyTree& tree,
                                  bool show_aliases) -> std::string {
  std::string output = tree.parent + '\n';
  auto nodes = tree.nodes;
  std::ranges::sort(nodes, [](const auto& left, const auto& right) {
    return left.canonical_key < right.canonical_key;
  });
  for (std::size_t index = 0U; index < nodes.size(); ++index) {
    RenderNode(nodes[index], {}, index + 1U == nodes.size(), show_aliases,
               output);
  }
  return output;
}

}  // namespace tracer::core::infrastructure::config::loader::detail
