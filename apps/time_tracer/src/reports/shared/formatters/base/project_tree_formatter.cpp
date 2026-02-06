// reports/shared/formatters/base/project_tree_formatter.cpp
#include "reports/shared/formatters/base/project_tree_formatter.hpp"

#include <algorithm>
#include <iomanip>
#include <stack>
#include <vector>

#include "reports/shared/utils/format/time_format.hpp"

namespace reporting {

// 定义栈帧结构，用于将递归转换为迭代
namespace {
struct StackFrame {
  const ProjectNode* node;
  int indent;
  // 存储指向子节点的指针，避免拷贝
  std::vector<const std::pair<const std::string, ProjectNode>*> sorted_children;
  size_t current_child_index = 0;
  bool list_started = false;
};
}  // namespace

ProjectTreeFormatter::ProjectTreeFormatter(
    std::unique_ptr<IFormattingStrategy> strategy)
    : m_strategy(std::move(strategy)) {
  if (!m_strategy) {
    throw std::invalid_argument("Formatting strategy cannot be null.");
  }
}

// Public API: keep parameter order and naming for ABI compatibility.
// NOLINTBEGIN(bugprone-easily-swappable-parameters)
auto ProjectTreeFormatter::FormatProjectTree(const ProjectTree& tree,
                                               long long total_duration,
                                               int avg_days) const
    -> std::string {
  std::stringstream output_ss;

  // [优化 1]：顶层节点也使用指针，避免深拷贝整个树
  using NodePair = std::pair<const std::string, ProjectNode>;
  std::vector<const NodePair*> sorted_top_level;
  sorted_top_level.reserve(tree.size());  // 预分配内存

  for (const auto& pair : tree) {
    sorted_top_level.push_back(&pair);
  }

  // 对指针进行排序
  std::ranges::sort(sorted_top_level,
                    [](const NodePair* lhs, const NodePair* rhs) -> bool {
                      return lhs->second.duration > rhs->second.duration;
                    });

  for (const auto* pair_ptr : sorted_top_level) {
    const std::string& category_name = pair_ptr->first;
    const ProjectNode& category_node = pair_ptr->second;

    double percentage = (total_duration > 0)
                            ? (static_cast<double>(category_node.duration) /
                               static_cast<double>(total_duration) * 100.0)
                            : 0.0;

    // 格式化分类标题
    output_ss << m_strategy->FormatCategoryHeader(
        category_name, TimeFormatDuration(category_node.duration, avg_days),
        percentage);

    // 调用迭代函数生成子树
    GenerateSortedOutput(output_ss, category_node, 0, avg_days);
  }

  return output_ss.str();
}
// NOLINTEND(bugprone-easily-swappable-parameters)

// [优化 2]：使用迭代（Stack）替代递归，防止深层级导致的栈溢出
// NOLINTBEGIN(bugprone-easily-swappable-parameters)
void ProjectTreeFormatter::GenerateSortedOutput(std::stringstream& output_ss,
                                                   const ProjectNode& root_node,
                                                   int root_indent,
                                                   int avg_days) const {
  // 如果根节点没有子节点，直接返回，避免建立栈的开销
  if (root_node.children.empty()) {
    return;
  }

  std::stack<StackFrame> stack;

  // 初始化根栈帧
  StackFrame root_frame;
  root_frame.node = &root_node;
  root_frame.indent = root_indent;

  // 预处理根节点的子节点排序
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

    // 1. 处理列表开始钩子 (对应递归前的逻辑)
    if (!frame.list_started) {
      output_ss << m_strategy->StartChildrenList();
      frame.list_started = true;
    }

    // 2. 遍历子节点
    bool pushed_new_frame = false;

    while (frame.current_child_index < frame.sorted_children.size()) {
      const auto* pair_ptr = frame.sorted_children[frame.current_child_index];
      const std::string& name = pair_ptr->first;
      const ProjectNode& child_node = pair_ptr->second;

      // 移动索引，准备下一次循环处理下一个兄弟节点
      frame.current_child_index++;

      if (child_node.duration > 0 || !child_node.children.empty()) {
        // 输出当前节点
        output_ss << m_strategy->FormatTreeNode(
            name, TimeFormatDuration(child_node.duration, avg_days),
            frame.indent);

        // 如果该节点有子节点，则压入新栈帧（模拟递归深入）
        if (!child_node.children.empty()) {
          StackFrame child_frame;
          child_frame.node = &child_node;
          child_frame.indent = frame.indent + 1;

          // 排序子节点的子节点
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
          break;  // 跳出内层循环，回到外层循环处理新的栈顶（即刚刚压入的子节点）
        }
      }
    }

    if (pushed_new_frame) {
      continue;
    }

    // 3. 处理列表结束钩子 (对应递归后的逻辑)
    output_ss << m_strategy->EndChildrenList();
    stack.pop();  // 弹出当前帧，回溯到上一层
  }
}
// NOLINTEND(bugprone-easily-swappable-parameters)

}  // namespace reporting
