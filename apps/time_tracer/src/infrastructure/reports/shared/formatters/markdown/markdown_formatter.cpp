// infrastructure/reports/shared/formatters/markdown/markdown_formatter.cpp
#include "infrastructure/reports/shared/formatters/markdown/markdown_formatter.hpp"

#include <iomanip>
#include <memory>
#include <sstream>

#include "infrastructure/reports/shared/formatters/base/project_tree_formatter.hpp"

namespace MarkdownFormatter {

/**
 * @class MarkdownFormattingStrategy
 * @brief 实现了 IFormattingStrategy 接口，用于生成 Markdown 格式的字符串。
 */
class MarkdownFormattingStrategy : public reporting::IFormattingStrategy {
 public:
  [[nodiscard]] auto FormatCategoryHeader(const std::string& category_name,
                                          const std::string& formatted_duration,
                                          double percentage) const
      -> std::string override {
    std::stringstream output_ss;
    // [修改] 将 ## 改为 ###
    output_ss << "\n### " << category_name << ": " << formatted_duration << " ("
              << std::fixed << std::setprecision(1) << percentage << "%) ###\n";
    return output_ss.str();
  }
  [[nodiscard]] auto FormatTreeNode(const std::string& project_name,
                                    const std::string& formatted_duration,
                                    int indent_level) const
      -> std::string override {
    std::stringstream output_ss;
    constexpr int kIndentMultiplier = 2;
    output_ss << std::string(static_cast<size_t>(indent_level) *
                                 static_cast<size_t>(kIndentMultiplier),
                             ' ')
              << "- " << project_name << ": " << formatted_duration << "\n";
    return output_ss.str();
  }
};

// --- Public API ---

// [修正] 添加 reporting:: 命名空间前缀
// Public API: keep parameter order and naming for ABI compatibility.
// NOLINTBEGIN(bugprone-easily-swappable-parameters)
auto FormatProjectTree(const reporting::ProjectTree& tree,
                       long long total_duration, int avg_days) -> std::string {
  auto strategy = std::make_unique<MarkdownFormattingStrategy>();
  reporting::ProjectTreeFormatter formatter(std::move(strategy));
  return formatter.FormatProjectTree(tree, total_duration, avg_days);
}
// NOLINTEND(bugprone-easily-swappable-parameters)

}  // namespace MarkdownFormatter
