// reports/shared/formatters/markdown/markdown_formatter.cpp
#include "markdown_formatter.hpp"

#include <iomanip>
#include <memory>
#include <sstream>

#include "reports/shared/formatters/base/project_tree_formatter.hpp"

namespace MarkdownFormatter {

/**
 * @class MarkdownFormattingStrategy
 * @brief 实现了 IFormattingStrategy 接口，用于生成 Markdown 格式的字符串。
 */
class MarkdownFormattingStrategy : public reporting::IFormattingStrategy {
 public:
  [[nodiscard]] auto format_category_header(
      const std::string& category_name, const std::string& formatted_duration,
      double percentage) const -> std::string override {
    std::stringstream ss;
    // [修改] 将 ## 改为 ###
    ss << "\n### " << category_name << ": " << formatted_duration << " ("
       << std::fixed << std::setprecision(1) << percentage << "%) ###\n";
    return ss.str();
  }
  [[nodiscard]] auto format_tree_node(const std::string& project_name,
                                      const std::string& formatted_duration,
                                      int indent_level) const
      -> std::string override {
    std::stringstream ss;
    ss << std::string(indent_level * 2, ' ') << "- " << project_name << ": "
       << formatted_duration << "\n";
    return ss.str();
  }
};

// --- Public API ---

// [修正] 添加 reporting:: 命名空间前缀
auto format_project_tree(const reporting::ProjectTree& tree,
                         long long total_duration, int avg_days)
    -> std::string {
  auto strategy = std::make_unique<MarkdownFormattingStrategy>();
  reporting::ProjectTreeFormatter formatter(std::move(strategy));
  return formatter.format_project_tree(tree, total_duration, avg_days);
}

}  // namespace MarkdownFormatter