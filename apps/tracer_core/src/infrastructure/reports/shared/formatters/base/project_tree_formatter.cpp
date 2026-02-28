// infrastructure/reports/shared/formatters/base/project_tree_formatter.cpp
#include "infrastructure/reports/shared/formatters/base/project_tree_formatter.hpp"

#include <algorithm>
#include <cstdint>
#include <stack>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "infrastructure/reports/shared/utils/format/time_format.hpp"

namespace reporting {
namespace {

struct StackFrame {
  int indent = 0;
  std::vector<const std::pair<const std::string, ProjectNode>*> sorted_children;
  size_t current_child_index = 0;
  bool list_started = false;
};

struct FlatProjectTreeNode {
  std::string_view name;
  int64_t duration = 0;
  int32_t parent_index = -1;
  std::vector<uint32_t> children;
};

struct FlatStackFrame {
  int indent = 0;
  std::vector<uint32_t> sorted_children;
  size_t current_child_index = 0;
  bool list_started = false;
};

auto CalculatePercentage(int64_t duration, long long total_duration) -> double {
  if (total_duration <= 0) {
    return 0.0;
  }
  return static_cast<double>(duration) / static_cast<double>(total_duration) *
         100.0;
}

}  // namespace

ProjectTreeFormatter::ProjectTreeFormatter(
    std::unique_ptr<IFormattingStrategy> strategy)
    : m_strategy_(std::move(strategy)) {
  if (!m_strategy_) {
    throw std::invalid_argument("Formatting strategy cannot be null.");
  }
}

// Public API: keep parameter order and naming for ABI compatibility.
// NOLINTBEGIN(bugprone-easily-swappable-parameters)
auto ProjectTreeFormatter::FormatProjectTree(const ProjectTree& tree,
                                             long long total_duration,
                                             int avg_days) const
    -> std::string {
  std::string output;

  using NodePair = std::pair<const std::string, ProjectNode>;
  std::vector<const NodePair*> sorted_top_level;
  sorted_top_level.reserve(tree.size());

  for (const auto& pair : tree) {
    sorted_top_level.push_back(&pair);
  }

  std::ranges::sort(sorted_top_level,
                    [](const NodePair* lhs, const NodePair* rhs) -> bool {
                      return lhs->second.duration > rhs->second.duration;
                    });

  for (const auto* pair_ptr : sorted_top_level) {
    const std::string& category_name = pair_ptr->first;
    const ProjectNode& category_node = pair_ptr->second;

    output += m_strategy_->FormatCategoryHeader(
        category_name, TimeFormatDuration(category_node.duration, avg_days),
        CalculatePercentage(category_node.duration, total_duration));

    GenerateSortedOutput(output, category_node, 0, avg_days);
  }

  return output;
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
auto ProjectTreeFormatter::FormatProjectTree(const TtProjectTreeNodeV1* nodes,
                                             uint32_t node_count,
                                             long long total_duration,
                                             int avg_days) const
    -> std::string {
  if ((nodes == nullptr) || (node_count == 0U)) {
    return "";
  }

  std::vector<FlatProjectTreeNode> parsed_nodes;
  parsed_nodes.reserve(node_count);

  for (uint32_t node_index = 0; node_index < node_count; ++node_index) {
    const TtProjectTreeNodeV1& node_view = nodes[node_index];
    std::string_view name_view;
    if ((node_view.name.data != nullptr) && (node_view.name.length > 0U)) {
      name_view = std::string_view(node_view.name.data,
                                   static_cast<size_t>(node_view.name.length));
    }

    FlatProjectTreeNode node{};
    node.name = name_view;
    node.duration = node_view.duration;
    node.parent_index = node_view.parentIndex;
    parsed_nodes.push_back(node);
  }

  std::vector<uint32_t> root_indices;
  root_indices.reserve(node_count);

  for (uint32_t node_index = 0; node_index < node_count; ++node_index) {
    const int32_t kParentIndex = parsed_nodes[node_index].parent_index;
    if ((kParentIndex < 0) ||
        std::cmp_greater_equal(kParentIndex, node_count) ||
        std::cmp_greater_equal(kParentIndex, node_index)) {
      root_indices.push_back(node_index);
      continue;
    }
    parsed_nodes[static_cast<size_t>(kParentIndex)].children.push_back(
        node_index);
  }

  std::ranges::sort(root_indices, [&](uint32_t lhs, uint32_t rhs) -> bool {
    return parsed_nodes[lhs].duration > parsed_nodes[rhs].duration;
  });

  std::string output;
  for (uint32_t root_index : root_indices) {
    const FlatProjectTreeNode& root_node = parsed_nodes[root_index];
    if (root_node.name.empty()) {
      continue;
    }

    output += m_strategy_->FormatCategoryHeader(
        std::string(root_node.name),
        TimeFormatDuration(root_node.duration, avg_days),
        CalculatePercentage(root_node.duration, total_duration));

    if (root_node.children.empty()) {
      continue;
    }

    std::stack<FlatStackFrame> stack;
    FlatStackFrame root_frame{};
    root_frame.indent = 0;
    root_frame.sorted_children = root_node.children;
    std::ranges::sort(
        root_frame.sorted_children, [&](uint32_t lhs, uint32_t rhs) -> bool {
          return parsed_nodes[lhs].duration > parsed_nodes[rhs].duration;
        });
    stack.push(std::move(root_frame));

    while (!stack.empty()) {
      FlatStackFrame& frame = stack.top();

      if (!frame.list_started) {
        output += m_strategy_->StartChildrenList();
        frame.list_started = true;
      }

      bool pushed_child_frame = false;
      while (frame.current_child_index < frame.sorted_children.size()) {
        const uint32_t kChildIndex =
            frame.sorted_children[frame.current_child_index];
        frame.current_child_index++;

        const FlatProjectTreeNode& child_node = parsed_nodes[kChildIndex];
        if ((child_node.duration <= 0) && child_node.children.empty()) {
          continue;
        }
        if (child_node.name.empty()) {
          continue;
        }

        output += m_strategy_->FormatTreeNode(
            std::string(child_node.name),
            TimeFormatDuration(child_node.duration, avg_days), frame.indent);

        if (!child_node.children.empty()) {
          FlatStackFrame child_frame{};
          child_frame.indent = frame.indent + 1;
          child_frame.sorted_children = child_node.children;
          std::ranges::sort(child_frame.sorted_children,
                            [&](uint32_t lhs, uint32_t rhs) -> bool {
                              return parsed_nodes[lhs].duration >
                                     parsed_nodes[rhs].duration;
                            });
          stack.push(std::move(child_frame));
          pushed_child_frame = true;
          break;
        }
      }

      if (pushed_child_frame) {
        continue;
      }

      output += m_strategy_->EndChildrenList();
      stack.pop();
    }
  }

  return output;
}
// NOLINTEND(bugprone-easily-swappable-parameters)

// NOLINTBEGIN(bugprone-easily-swappable-parameters)
void ProjectTreeFormatter::GenerateSortedOutput(std::string& output,
                                                const ProjectNode& root_node,
                                                int root_indent,
                                                int avg_days) const {
  if (root_node.children.empty()) {
    return;
  }

  std::stack<StackFrame> stack;
  StackFrame root_frame{};
  root_frame.indent = root_indent;

  using ChildPair = std::pair<const std::string, ProjectNode>;
  root_frame.sorted_children.reserve(root_node.children.size());
  for (const auto& pair : root_node.children) {
    root_frame.sorted_children.push_back(&pair);
  }
  std::ranges::sort(root_frame.sorted_children,
                    [](const ChildPair* lhs, const ChildPair* rhs) -> bool {
                      return lhs->second.duration > rhs->second.duration;
                    });
  stack.push(std::move(root_frame));

  while (!stack.empty()) {
    StackFrame& frame = stack.top();

    if (!frame.list_started) {
      output += m_strategy_->StartChildrenList();
      frame.list_started = true;
    }

    bool pushed_new_frame = false;
    while (frame.current_child_index < frame.sorted_children.size()) {
      const auto* pair_ptr = frame.sorted_children[frame.current_child_index];
      frame.current_child_index++;

      const std::string& name = pair_ptr->first;
      const ProjectNode& child_node = pair_ptr->second;
      if ((child_node.duration <= 0) && child_node.children.empty()) {
        continue;
      }

      output += m_strategy_->FormatTreeNode(
          name, TimeFormatDuration(child_node.duration, avg_days),
          frame.indent);

      if (!child_node.children.empty()) {
        StackFrame child_frame{};
        child_frame.indent = frame.indent + 1;

        child_frame.sorted_children.reserve(child_node.children.size());
        for (const auto& child_pair : child_node.children) {
          child_frame.sorted_children.push_back(&child_pair);
        }
        std::ranges::sort(
            child_frame.sorted_children,
            [](const ChildPair* lhs, const ChildPair* rhs) -> bool {
              return lhs->second.duration > rhs->second.duration;
            });

        stack.push(std::move(child_frame));
        pushed_new_frame = true;
        break;
      }
    }

    if (pushed_new_frame) {
      continue;
    }

    output += m_strategy_->EndChildrenList();
    stack.pop();
  }
}
// NOLINTEND(bugprone-easily-swappable-parameters)

}  // namespace reporting
