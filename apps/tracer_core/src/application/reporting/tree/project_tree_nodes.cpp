// application/reporting/tree/project_tree_nodes.cpp
#include "application/reporting/tree/project_tree_nodes.hpp"

#include <algorithm>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace tracer_core::application::reporting::tree {
namespace {

struct NamedReportNodeRef {
  std::string_view name;
  const ::reporting::ProjectNode* node = nullptr;
};

[[nodiscard]] auto CollectSortedReportChildren(
    const ::reporting::ProjectNode& node) -> std::vector<NamedReportNodeRef> {
  std::vector<NamedReportNodeRef> children;
  children.reserve(node.children.size());
  for (const auto& [name, child] : node.children) {
    children.push_back({.name = name, .node = &child});
  }
  std::ranges::sort(
      children,
      [](const NamedReportNodeRef& lhs, const NamedReportNodeRef& rhs) -> bool {
        return lhs.name < rhs.name;
      });
  return children;
}

[[nodiscard]] auto JoinTreePath(std::string_view parent_path,
                                std::string_view name) -> std::string {
  if (parent_path.empty()) {
    return std::string(name);
  }
  std::string out;
  out.reserve(parent_path.size() + 1 + name.size());
  out.append(parent_path);
  out.push_back('_');
  out.append(name);
  return out;
}

[[nodiscard]] auto BuildNodeFromReportNode(std::string_view name,
                                           const ::reporting::ProjectNode& node,
                                           std::string_view parent_path)
    -> ProjectTreeNode {
  ProjectTreeNode out{};
  out.name = std::string(name);
  out.path = JoinTreePath(parent_path, name);
  out.duration_seconds = node.duration;

  const auto children = CollectSortedReportChildren(node);
  out.children.reserve(children.size());
  for (const auto& child : children) {
    out.children.push_back(
        BuildNodeFromReportNode(child.name, *child.node, out.path));
  }
  return out;
}

[[nodiscard]] auto ResolveNodePath(const ProjectTreeNode& node,
                                   std::string_view parent_path)
    -> std::string {
  if (!node.path.empty()) {
    return node.path;
  }
  return JoinTreePath(parent_path, node.name);
}

auto CollectTreeMatchesByPath(const ProjectTreeNode& node,
                              std::string_view parent_path,
                              std::string_view root_pattern,
                              std::vector<ProjectTreeNode>& out) -> void {
  const std::string current_path = ResolveNodePath(node, parent_path);
  if (current_path == root_pattern) {
    out.push_back(node);
  }
  for (const auto& child : node.children) {
    CollectTreeMatchesByPath(child, current_path, root_pattern, out);
  }
}

[[nodiscard]] auto CloneNodeWithDepthLimit(const ProjectTreeNode& node,
                                           int current_depth, int max_depth)
    -> ProjectTreeNode {
  ProjectTreeNode out = node;
  out.children.clear();

  if (max_depth < 0 || current_depth < max_depth) {
    out.children.reserve(node.children.size());
    for (const auto& child : node.children) {
      out.children.push_back(
          CloneNodeWithDepthLimit(child, current_depth + 1, max_depth));
    }
  }
  return out;
}

}  // namespace

auto BuildProjectTreeNodesFromReportTree(const ::reporting::ProjectTree& tree)
    -> std::vector<ProjectTreeNode> {
  std::vector<NamedReportNodeRef> roots;
  roots.reserve(tree.size());
  for (const auto& [name, node] : tree) {
    roots.push_back({.name = name, .node = &node});
  }
  std::ranges::sort(
      roots,
      [](const NamedReportNodeRef& lhs, const NamedReportNodeRef& rhs) -> bool {
        return lhs.name < rhs.name;
      });

  std::vector<ProjectTreeNode> out;
  out.reserve(roots.size());
  for (const auto& root : roots) {
    out.push_back(BuildNodeFromReportNode(root.name, *root.node, ""));
  }
  return out;
}

auto FindProjectTreeNodesByPath(const std::vector<ProjectTreeNode>& roots,
                                std::string_view root_pattern)
    -> std::vector<ProjectTreeNode> {
  if (root_pattern.empty()) {
    return roots;
  }

  std::vector<ProjectTreeNode> out;
  for (const auto& root : roots) {
    CollectTreeMatchesByPath(root, "", root_pattern, out);
  }
  return out;
}

auto LimitProjectTreeDepth(const std::vector<ProjectTreeNode>& roots,
                           int max_depth) -> std::vector<ProjectTreeNode> {
  if (max_depth < 0) {
    return roots;
  }

  std::vector<ProjectTreeNode> out;
  out.reserve(roots.size());
  for (const auto& root : roots) {
    out.push_back(CloneNodeWithDepthLimit(root, 0, max_depth));
  }
  return out;
}

}  // namespace tracer_core::application::reporting::tree
