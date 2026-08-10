// infra/insights/shared/formatters/markdown/markdown_formatter.cpp
#include "infra/insights/shared/formatters/markdown/markdown_formatter.hpp"

#include <cstdint>
#include <format>
#include <memory>
#include <string>

#include "infra/insights/shared/formatters/base/project_tree_formatter.hpp"
#include "infra/insights/shared/utils/format/time_format.hpp"

namespace MarkdownFormatter {

namespace {
constexpr std::size_t kDecimalOutputReserve = 24U;

auto FormatOneDecimal(double value) -> std::string {
  const auto kScaled = static_cast<std::int64_t>(
      (value >= 0.0) ? ((value * 10.0) + 0.5) : ((value * 10.0) - 0.5));
  std::int64_t abs_scaled = (kScaled < 0) ? -kScaled : kScaled;
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

auto FormatAverageOccurrenceCount(std::int64_t occurrence_count, int avg_days)
    -> std::string {
  if (avg_days <= 0) {
    return "0.00";
  }
  return std::format("{:.2f}", static_cast<double>(occurrence_count) /
                                   static_cast<double>(avg_days));
}

}  // namespace

/**
 * @class MarkdownFormattingStrategy
 * @brief 实现了 IFormattingStrategy 接口，用于生成 Markdown 格式的字符串。
 */
class MarkdownFormattingStrategy : public insights::IFormattingStrategy {
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
    output += "%)\n";
    return output;
  }

  [[nodiscard]] auto FormatCategoryHeader(
      const std::string& category_name,
      const std::string& /*formatted_duration*/, double percentage,
      std::int64_t duration_seconds, std::int64_t occurrence_count,
      int avg_days) const -> std::string override {
    std::string output;
    output += "- **";
    output += category_name;
    output += "**: ";
    output += TimeFormatDuration(duration_seconds);
    output += " (";
    output += FormatOneDecimal(percentage);
    output += "%)\n";
    if (occurrence_count > 0) {
      output += "  *Average: ";
      output += TimeFormatDuration(avg_days > 0 ? duration_seconds / avg_days
                                                : duration_seconds);
      output += "/day · ";
      output += std::to_string(occurrence_count);
      output += " times · ";
      output += FormatAverageOccurrenceCount(occurrence_count, avg_days);
      output += " times/day*\n";
    }
    return output;
  }
  [[nodiscard]] auto FormatTreeNode(const std::string& project_name,
                                    const std::string& formatted_duration,
                                    int indent_level) const
      -> std::string override {
    constexpr int kIndentMultiplier = 2;
    std::string output(static_cast<size_t>(indent_level + 1) *
                           static_cast<size_t>(kIndentMultiplier),
                       ' ');
    output += "- ";
    output += project_name;
    output += ": ";
    output += formatted_duration;
    output += "\n";
    return output;
  }

  [[nodiscard]] auto FormatTreeNode(const std::string& project_name,
                                    const std::string& formatted_duration,
                                    int indent_level, double percentage,
                                    std::int64_t duration_seconds,
                                    std::int64_t occurrence_count,
                                    int avg_days) const
      -> std::string override {
    constexpr int kIndentMultiplier = 2;
    const auto kActivityIndent = static_cast<size_t>(indent_level + 1) *
                                 static_cast<size_t>(kIndentMultiplier);
    std::string output(kActivityIndent, ' ');
    output += "- ";
    output += project_name;
    output += ": ";
    output += formatted_duration;
    output += " (";
    output += FormatOneDecimal(percentage);
    output += "%)\n";
    if (occurrence_count > 0) {
      output.append(kActivityIndent + static_cast<size_t>(kIndentMultiplier),
                    ' ');
      output += "*Average: ";
      output += TimeFormatDuration(avg_days > 0 ? duration_seconds / avg_days
                                                : duration_seconds);
      output += "/day · ";
      output += std::to_string(occurrence_count);
      output += " times · ";
      output += FormatAverageOccurrenceCount(occurrence_count, avg_days);
      output += " times/day*\n";
    }
    return output;
  }
};

// --- Public API ---

// [修正] 添加 insights:: 命名空间前缀
// Public API: keep parameter order and naming for ABI compatibility.
// NOLINTBEGIN(bugprone-easily-swappable-parameters)
auto FormatProjectTree(const insights::ProjectTree& tree,
                       std::int64_t total_duration, int avg_days)
    -> std::string {
  auto strategy = std::make_unique<MarkdownFormattingStrategy>();
  insights::ProjectTreeFormatter formatter(std::move(strategy));
  return formatter.FormatProjectTree(tree, total_duration, avg_days);
}
// NOLINTEND(bugprone-easily-swappable-parameters)

}  // namespace MarkdownFormatter
