// infrastructure/reports/shared/formatters/markdown/markdown_formatter.cpp
#include "infrastructure/reports/shared/formatters/markdown/markdown_formatter.hpp"

#include <memory>
#include <string>

#include "infrastructure/reports/shared/formatters/base/project_tree_formatter.hpp"

namespace MarkdownFormatter {

namespace {
constexpr std::size_t kDecimalOutputReserve = 24U;

auto FormatOneDecimal(double value) -> std::string {
  const auto kScaled = static_cast<long long>(
      (value >= 0.0) ? ((value * 10.0) + 0.5) : ((value * 10.0) - 0.5));
  long long abs_scaled = (kScaled < 0) ? -kScaled : kScaled;
  const auto kWholePart = abs_scaled / 10;
  const auto kFractionalPart = abs_scaled % 10;

  std::string output;
  output.reserve(kDecimalOutputReserve);
  if (kScaled < 0) {
    output.push_back('-');
  }
  output += std::to_string(kWholePart);
  output.push_back('.');
  output += std::to_string(kFractionalPart);
  return output;
}

}  // namespace

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
    std::string output;
    // [修改] 将 ## 改为 ###
    output += "\n### ";
    output += category_name;
    output += ": ";
    output += formatted_duration;
    output += " (";
    output += FormatOneDecimal(percentage);
    output += "%) ###\n";
    return output;
  }
  [[nodiscard]] auto FormatTreeNode(const std::string& project_name,
                                    const std::string& formatted_duration,
                                    int indent_level) const
      -> std::string override {
    constexpr int kIndentMultiplier = 2;
    std::string output(static_cast<size_t>(indent_level) *
                           static_cast<size_t>(kIndentMultiplier),
                       ' ');
    output += "- ";
    output += project_name;
    output += ": ";
    output += formatted_duration;
    output += "\n";
    return output;
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

auto FormatProjectTree(const TtProjectTreeNodeV1* nodes, uint32_t node_count,
                       long long total_duration, int avg_days) -> std::string {
  auto strategy = std::make_unique<MarkdownFormattingStrategy>();
  reporting::ProjectTreeFormatter formatter(std::move(strategy));
  return formatter.FormatProjectTree(nodes, node_count, total_duration,
                                     avg_days);
}
// NOLINTEND(bugprone-easily-swappable-parameters)

}  // namespace MarkdownFormatter
