// api/cli/impl/utils/tree_formatter.hpp
#ifndef CLI_IMPL_UTILS_TREE_FORMATTER_H_
#define CLI_IMPL_UTILS_TREE_FORMATTER_H_

#include <string>
#include <vector>

#include "application/reporting/tree/project_tree_data.hpp"

/**
 * @brief 项目树输出格式化器，负责将树数据结构输出到控制台
 */
class TreeFormatter {
 public:
  /**
   * @brief 输出根项目列表
   */
  static auto PrintRoots(const std::vector<std::string>& roots) -> void;

  /**
   * @brief 输出树形结构
   */
  static auto PrintTree(const std::vector<ProjectTreeNode>& nodes) -> void;
};

#endif  // CLI_IMPL_UTILS_TREE_FORMATTER_H_
