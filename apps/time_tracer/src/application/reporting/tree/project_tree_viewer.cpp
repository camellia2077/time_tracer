// application/reporting/tree/project_tree_viewer.cpp
#include "application/reporting/tree/project_tree_viewer.hpp"

#include <algorithm>
#include <map>
#include <optional>
#include <queue>
#include <string>
#include <vector>

#include "domain/repositories/i_project_repository.hpp"

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

auto FindNodesByPath(const std::vector<InternalNode*>& roots,
                     const std::string& root_pattern)
    -> std::vector<InternalNode*> {
  std::vector<InternalNode*> target_nodes;
  std::queue<std::pair<InternalNode*, std::string>> search_queue;
  for (auto* root_node : roots) {
    search_queue.emplace(root_node, root_node->name);
  }

  while (!search_queue.empty()) {
    auto [current_node, current_path] = search_queue.front();
    search_queue.pop();

    if (current_path == root_pattern) {
      target_nodes.push_back(current_node);
    }

    for (auto* child : current_node->children) {
      search_queue.emplace(child, current_path + "_" + child->name);
    }
  }
  return target_nodes;
}

// 将 InternalNode 转换为 ProjectTreeNode，并应用深度限制
auto ConvertToTreeNode(const InternalNode* node, int current_depth,
                       int max_depth) -> ProjectTreeNode {
  ProjectTreeNode result;
  result.name = node->name;

  if (max_depth < 0 || current_depth < max_depth) {
    for (const auto* child : node->children) {
      result.children.push_back(
          ConvertToTreeNode(child, current_depth + 1, max_depth));
    }
  }
  return result;
}

}  // namespace

ProjectTreeViewer::ProjectTreeViewer(
    std::shared_ptr<IProjectRepository> repository)
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

  std::vector<InternalNode*> target_roots;
  if (root_pattern.empty()) {
    target_roots = roots;
  } else {
    target_roots = FindNodesByPath(roots, root_pattern);

    if (target_roots.empty()) {
      return std::nullopt;  // 未找到匹配的节点
    }
  }

  std::vector<ProjectTreeNode> result;
  for (auto* root : target_roots) {
    result.push_back(ConvertToTreeNode(root, 0, max_depth));
  }
  return result;
}
