// application/reporting/tree/project_tree_viewer.hpp
#ifndef APPLICATION_REPORTING_TREE_PROJECT_TREE_VIEWER_H_
#define APPLICATION_REPORTING_TREE_PROJECT_TREE_VIEWER_H_

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "application/reporting/tree/project_tree_data.hpp"

class IProjectRepository;

/**
 * @brief 项目树查询服务，返回数据结构而非直接输出
 */
class ProjectTreeViewer {
 public:
  explicit ProjectTreeViewer(std::shared_ptr<IProjectRepository> repository);
  ~ProjectTreeViewer();

  /**
   * @brief 获取所有根项目名称
   * @return 根项目名称列表
   */
  [[nodiscard]] auto GetRoots() -> std::vector<std::string>;

  /**
   * @brief 获取项目树结构
   * @param root_pattern 根节点路径过滤（如 "study_math"），为空则返回所有根节点
   * @param max_depth 最大深度，-1 表示无限制
   * @return 匹配的树节点列表，若未找到则返回空（调用者应检查）
   */
  [[nodiscard]] auto GetTree(const std::string& root_pattern, int max_depth)
      -> std::optional<std::vector<ProjectTreeNode>>;

 private:
  std::shared_ptr<IProjectRepository> repository_;
};

#endif  // APPLICATION_REPORTING_TREE_PROJECT_TREE_VIEWER_H_
