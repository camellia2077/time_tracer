// domain/insights/models/project_tree.hpp
#ifndef DOMAIN_INSIGHTS_MODELS_PROJECT_TREE_H_
#define DOMAIN_INSIGHTS_MODELS_PROJECT_TREE_H_

#include <cstdint>
#include <string>
#include <unordered_map>

namespace insights {

struct ProjectNode {
  std::int64_t duration = 0;
  std::int64_t occurrence_count = 0;
  // 使用 unordered_map 提升构建速度
  // 在生成报告时（如 ProjectTreeFormatter），
  // 程序显式地按 duration（时长）进行了重新排序。
  // 这意味着 std::map 在构建时的排序工作是完全浪费的。
  // 改用 std::unordered_map
  // 插入和查找的平均时间复杂度从 O(log N) 降低到 O(1)。
  std::unordered_map<std::string, ProjectNode> children;
};

using ProjectTree = std::unordered_map<std::string, ProjectNode>;

}  // namespace insights

#endif  // DOMAIN_INSIGHTS_MODELS_PROJECT_TREE_H_
