// reports/shared/formatters/base/project_tree_formatter.hpp
#ifndef REPORTS_SHARED_FORMATTERS_BASE_PROJECT_TREE_FORMATTER_H_
#define REPORTS_SHARED_FORMATTERS_BASE_PROJECT_TREE_FORMATTER_H_

#include <memory>
#include <sstream>
#include <string>

#include "reports/data/model/project_tree.hpp"
#include "reports/shared/api/shared_api.hpp"

namespace reporting {

/**
 * @class IFormattingStrategy
 * @brief 定义了将项目树节点格式化为特定字符串的接口（策略接口）。
 * 将内存中抽象的 项目树数据结构 (ProjectTree)
 * 转换成用户最终看到的 格式化文本（Markdown列表、Typst代码 或 LaTeX代码）
 */
class REPORTS_SHARED_API IFormattingStrategy {
 public:
  virtual ~IFormattingStrategy() = default;

  virtual std::string format_category_header(
      const std::string& category_name, const std::string& formatted_duration,
      double percentage) const = 0;

  virtual std::string format_tree_node(const std::string& project_name,
                                       const std::string& formatted_duration,
                                       int indent_level) const = 0;

  virtual std::string start_children_list() const { return ""; }
  virtual std::string end_children_list() const { return ""; }
};

/**
 * @class ProjectTreeFormatter
 * @brief 负责遍历项目树并使用指定的格式化策略来生成报告字符串。
 */
class REPORTS_SHARED_API ProjectTreeFormatter {  // [新增] 必须添加 API 宏
 public:
  /**
   * @brief 构造一个新的 ProjectTreeFormatter。
   */
  explicit ProjectTreeFormatter(std::unique_ptr<IFormattingStrategy> strategy);

  // Public API: keep parameter order and naming for ABI compatibility.
  // NOLINTBEGIN(bugprone-easily-swappable-parameters)
  std::string format_project_tree(const ProjectTree& tree,
                                  long long total_duration, int avg_days) const;
  // NOLINTEND(bugprone-easily-swappable-parameters)

 private:
  std::unique_ptr<IFormattingStrategy> m_strategy;

  // NOLINTBEGIN(bugprone-easily-swappable-parameters)
  void generate_sorted_output(std::stringstream& ss, const ProjectNode& node,
                              int indent, int avg_days) const;
  // NOLINTEND(bugprone-easily-swappable-parameters)
};

}  // namespace reporting

#endif  // REPORTS_SHARED_FORMATTERS_BASE_PROJECT_TREE_FORMATTER_H_
