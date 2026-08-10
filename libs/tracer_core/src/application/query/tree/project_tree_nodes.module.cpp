module;

#include <algorithm>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

module tracer.core.application.query.tree.nodes;

import tracer.core.application.query.tree.data;
import tracer.core.domain.insights.models.project_tree;

namespace tracer::core::application::query::tree {
namespace {

using tracer::core::domain::modinsights::ProjectNode;
using tracer::core::domain::modinsights::ProjectTree;

struct NamedInsightsNodeRef {
  std::string_view name;
  const ProjectNode* node = nullptr;
};

[[nodiscard]] auto CollectSortedInsightsChildren(const ProjectNode& node)
    -> std::vector<NamedInsightsNodeRef> {
  std::vector<NamedInsightsNodeRef> children;
  children.reserve(node.children.size());
  for (const auto& [name, child] : node.children) {
    children.push_back({.name = name, .node = &child});
  }
  std::ranges::sort(
      children,
      [](const NamedInsightsNodeRef& lhs, const NamedInsightsNodeRef& rhs) -> bool {
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

[[nodiscard]] auto BuildNodeFromInsightsNode(
    std::string_view name, const ProjectNode& node,
    std::string_view parent_path, std::optional<long long> parent_duration)
    -> ProjectTreeNode {
  ProjectTreeNode out{};
  out.name = std::string(name);
  out.path = JoinTreePath(parent_path, name);
  out.duration_seconds = node.duration;
  if (parent_duration.has_value() && *parent_duration > 0) {
    out.parent_duration_percent = (static_cast<double>(node.duration) * 100.0) /
                                  static_cast<double>(*parent_duration);
  }

  const auto kChildren = CollectSortedInsightsChildren(node);
  out.children.reserve(kChildren.size());
  for (const auto& child : kChildren) {
    out.children.push_back(BuildNodeFromInsightsNode(child.name, *child.node,
                                                   out.path, node.duration));
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
                              std::vector<ProjectTreeNode>& out,
                              std::string_view root_pattern) -> void {
  const std::string kCurrentPath = ResolveNodePath(node, parent_path);
  if (kCurrentPath == root_pattern) {
    out.push_back(node);
  }
  for (const auto& child : node.children) {
    CollectTreeMatchesByPath(child, kCurrentPath, out, root_pattern);
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

auto BuildProjectTreeNodesFromInsightsTree(const ProjectTree& tree)
    -> std::vector<ProjectTreeNode> {
  std::vector<NamedInsightsNodeRef> roots;
  roots.reserve(tree.size());
  for (const auto& [name, node] : tree) {
    roots.push_back({.name = name, .node = &node});
  }
  std::ranges::sort(
      roots,
      [](const NamedInsightsNodeRef& lhs, const NamedInsightsNodeRef& rhs) -> bool {
        return lhs.name < rhs.name;
      });

  std::vector<ProjectTreeNode> out;
  out.reserve(roots.size());
  for (const auto& root : roots) {
    out.push_back(
        BuildNodeFromInsightsNode(root.name, *root.node, "", std::nullopt));
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
    CollectTreeMatchesByPath(root, "", out, root_pattern);
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

}  // namespace tracer::core::application::query::tree
