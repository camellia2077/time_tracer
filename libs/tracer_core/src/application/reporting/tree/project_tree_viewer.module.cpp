module;

#include <map>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "domain/repositories/i_project_repository.hpp"

module tracer.core.application.reporting.tree.viewer;

import tracer.core.application.reporting.tree.data;
import tracer.core.application.reporting.tree.nodes;

namespace tracer::core::application::reporting::tree {
namespace {

struct InternalNode {
  int id;
  std::optional<int> parent_id;
  std::string name;
  std::vector<InternalNode*> children;
};

auto BuildInternalTree(const std::vector<ProjectEntity>& projects,
                       std::vector<InternalNode>& all_nodes,
                       std::vector<InternalNode*>& root_nodes) -> void {
  all_nodes.reserve(projects.size());
  for (const auto& project : projects) {
    InternalNode node;
    node.id = project.id;
    node.parent_id = project.parent_id;
    node.name = project.name;
    all_nodes.push_back(node);
  }

  std::map<int, InternalNode*> id_to_node_map;
  for (auto& node : all_nodes) {
    id_to_node_map[node.id] = &node;
  }

  for (auto& node : all_nodes) {
    if (node.parent_id.has_value()) {
      auto iterator = id_to_node_map.find(*node.parent_id);
      if (iterator != id_to_node_map.end()) {
        iterator->second->children.push_back(&node);
      }
    } else {
      root_nodes.push_back(&node);
    }
  }
}

[[nodiscard]] auto ConvertToTreeNode(const InternalNode* node,
                                     std::string current_path)
    -> ProjectTreeNode {
  ProjectTreeNode result;
  result.name = node->name;
  result.path = std::move(current_path);

  for (const auto* child : node->children) {
    std::string child_path = result.path;
    child_path.push_back('_');
    child_path += child->name;
    result.children.push_back(ConvertToTreeNode(child, std::move(child_path)));
  }
  return result;
}

}  // namespace

ProjectTreeViewer::ProjectTreeViewer(ProjectRepositoryPtr repository)
    : repository_(std::move(repository)) {}

ProjectTreeViewer::~ProjectTreeViewer() = default;

auto ProjectTreeViewer::GetRoots() -> std::vector<std::string> {
  auto projects = repository_->GetAllProjects();

  std::vector<std::string> roots;
  for (const auto& project : projects) {
    if (!project.parent_id.has_value()) {
      roots.push_back(project.name);
    }
  }
  return roots;
}

auto ProjectTreeViewer::GetTree(const std::string& root_pattern, int max_depth)
    -> std::optional<std::vector<ProjectTreeNode>> {
  auto projects = repository_->GetAllProjects();

  std::vector<InternalNode> all_nodes;
  std::vector<InternalNode*> roots;
  BuildInternalTree(projects, all_nodes, roots);

  std::vector<ProjectTreeNode> full_tree_roots;
  full_tree_roots.reserve(roots.size());
  for (const auto* root : roots) {
    full_tree_roots.push_back(ConvertToTreeNode(root, root->name));
  }

  auto selected_roots = root_pattern.empty()
                            ? full_tree_roots
                            : FindProjectTreeNodesByPath(full_tree_roots,
                                                         root_pattern);
  if (selected_roots.empty()) {
    return std::nullopt;
  }

  return LimitProjectTreeDepth(selected_roots, max_depth);
}

}  // namespace tracer::core::application::reporting::tree
