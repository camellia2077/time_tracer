// reports/shared/formatters/typst/typ_utils.cpp
#include "reports/shared/formatters/typst/typ_utils.hpp"

#include <format>
#include <memory>
#include <sstream>

#include "reports/shared/formatters/base/project_tree_formatter.hpp"

namespace TypUtils {

class TypstFormattingStrategy : public reporting::IFormattingStrategy {
 public:
  TypstFormattingStrategy(std::string font, int font_size)
      : m_font_(std::move(font)), m_font_size_(font_size) {}

  [[nodiscard]] auto FormatCategoryHeader(
      const std::string& category_name, const std::string& formatted_duration,
      double percentage) const -> std::string override {
    // [核心修改] 将 = (Level 1) 改为 == (Level 2)
    // 这样它们就会成为 "Project Breakdown" 的子项
    return std::format(R"(#text(font: "{}", size: {}pt)[== {}])", m_font_,
                       m_font_size_,
                       std::format("{}: {} ({:.1f}%)", category_name,
                                   formatted_duration, percentage)) +
           "\n";
  }

  [[nodiscard]] auto FormatTreeNode(const std::string& project_name,
                                       const std::string& formatted_duration,
                                       int indent_level) const
      -> std::string override {
    constexpr int kIndentMultiplier = 2;
    return std::string(static_cast<size_t>(indent_level) *
                           static_cast<size_t>(kIndentMultiplier),
                       ' ') +
           "+ " + project_name + ": " + formatted_duration + "\n";
  }

 private:
  std::string m_font_;
  int m_font_size_;
};

// --- Public API ---

// [修正] 添加 reporting:: 命名空间前缀
// Public API: keep parameter order and naming for ABI compatibility.
// NOLINTBEGIN(bugprone-easily-swappable-parameters)
auto FormatProjectTree(const reporting::ProjectTree& tree,
                         long long total_duration, int avg_days,
                         const std::string& category_title_font,
                         int category_title_font_size) -> std::string {
  auto strategy = std::make_unique<TypstFormattingStrategy>(
      category_title_font, category_title_font_size);
  reporting::ProjectTreeFormatter formatter(std::move(strategy));
  return formatter.FormatProjectTree(tree, total_duration, avg_days);
}
// NOLINTEND(bugprone-easily-swappable-parameters)

}  // namespace TypUtils
