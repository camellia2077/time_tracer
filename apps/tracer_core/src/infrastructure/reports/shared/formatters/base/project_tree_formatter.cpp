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
