// cli/impl/utils/tree_formatter.cpp
#include "cli/impl/utils/tree_formatter.hpp"

#include <iostream>

namespace {

auto PrintNodeRecursive(const ProjectTreeNode& node, const std::string& prefix,
                        bool is_last, int current_depth) -> void {
  std::cout << prefix;
  if (current_depth > 0) {
    std::cout << (is_last ? "└── " : "├── ");
  }

  std::cout << node.name << "\n";

  std::string new_prefix = prefix;
  if (current_depth > 0) {
    new_prefix += (is_last ? "    " : "│   ");
  }

  for (size_t i = 0; i < node.children.size(); ++i) {
    PrintNodeRecursive(node.children[i], new_prefix,
                       i == node.children.size() - 1, current_depth + 1);
  }
}

}  // namespace

auto TreeFormatter::PrintRoots(const std::vector<std::string>& roots) -> void {
  std::cout << "Root Projects:\n";
  for (const auto& root : roots) {
    std::cout << "- " << root << "\n";
  }
}

auto TreeFormatter::PrintTree(const std::vector<ProjectTreeNode>& nodes)
    -> void {
  for (const auto& node : nodes) {
    PrintNodeRecursive(node, "", true, 0);
  }
}
