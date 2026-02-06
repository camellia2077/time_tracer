// application/reporting/tree/project_tree_data.hpp
#ifndef APPLICATION_REPORTING_TREE_PROJECT_TREE_DATA_H_
#define APPLICATION_REPORTING_TREE_PROJECT_TREE_DATA_H_

#include <string>
#include <vector>

/**
 * @brief 项目树节点数据结构，用于从 Application 层向 CLI 层传递树结构
 */
struct ProjectTreeNode {
  std::string name;
  std::vector<ProjectTreeNode> children;
};

#endif  // APPLICATION_REPORTING_TREE_PROJECT_TREE_DATA_H_
