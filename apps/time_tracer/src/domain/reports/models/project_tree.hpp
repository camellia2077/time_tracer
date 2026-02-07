// domain/reports/models/project_tree.hpp
#ifndef REPORTS_DATA_MODEL_PROJECT_TREE_H_
#define REPORTS_DATA_MODEL_PROJECT_TREE_H_

#include <string>
#include <unordered_map>

namespace reporting {

struct ProjectNode {
  long long duration = 0;
  // 使用 unordered_map 提升构建速度
  // 在生成报告时（如 ProjectTreeFormatter），
  // 程序显式地按 duration（时长）进行了重新排序。
  // 这意味着 std::map 在构建时的排序工作是完全浪费的。
  // 改用 std::unordered_map
  // 插入和查找的平均时间复杂度从 O(log N) 降低到 O(1)。
  std::unordered_map<std::string, ProjectNode> children;
};

using ProjectTree = std::unordered_map<std::string, ProjectNode>;

}  // namespace reporting

#endif  // REPORTS_DATA_MODEL_PROJECT_TREE_H_