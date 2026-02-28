// infrastructure/reports/shared/formatters/typst/typ_utils.cpp
#include "infrastructure/reports/shared/formatters/typst/typ_utils.hpp"

#include <memory>

#include "infrastructure/reports/shared/formatters/base/project_tree_formatter.hpp"

namespace TypUtils {

namespace {
constexpr int kDecimalBase = 10;
constexpr std::size_t kDecimalOutputReserve = 24;
constexpr std::size_t kCategoryHeaderReservePadding = 64;
constexpr std::size_t kTextSetupReservePadding = 64;
constexpr std::size_t kTitleTextReservePadding = 32;

auto FormatCompactNumber(double value) -> std::string {
  std::string output = std::to_string(value);
  while (!output.empty() && output.back() == '0') {
    output.pop_back();
  }
  if (!output.empty() && output.back() == '.') {
    output.pop_back();
  }
  if ((output == "-0") || output.empty()) {
    return "0";
  }
  return output;
}

auto FormatOneDecimal(double value) -> std::string {
  const auto kScaled = static_cast<long long>(
      (value >= 0.0) ? ((value * 10.0) + 0.5) : ((value * 10.0) - 0.5));
  long long abs_scaled = (kScaled < 0) ? -kScaled : kScaled;
  const auto kWholePart = abs_scaled / kDecimalBase;
  const auto kFractionalPart = abs_scaled % kDecimalBase;

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

class TypstFormattingStrategy : public reporting::IFormattingStrategy {
 public:
  TypstFormattingStrategy(std::string font, int font_size)
      : font_(std::move(font)), font_size_(font_size) {}

  [[nodiscard]] auto FormatCategoryHeader(const std::string& category_name,
                                          const std::string& formatted_duration,
                                          double percentage) const
      -> std::string override {
    std::string output;
    output.reserve(font_.size() + category_name.size() +
                   formatted_duration.size() + kCategoryHeaderReservePadding);
    output += R"(#text(font: ")";
    output += font_;
    output += R"(", size: )";
    output += std::to_string(font_size_);
    output += R"(pt)[== )";
    output += category_name;
    output += ": ";
    output += formatted_duration;
    output += " (";
    output += FormatOneDecimal(percentage);
    output += "%)]\n";
    return output;
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
  std::string font_;
  int font_size_;
};

}  // namespace

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
auto BuildTextSetup(const std::string& base_font, int base_font_size,
                    double line_spacing_em) -> std::string {
  std::string output;
  output.reserve(base_font.size() + kTextSetupReservePadding);
  output += R"(#set text(font: ")";
  output += base_font;
  output += R"(", size: )";
  output += std::to_string(base_font_size);
  output += R"(pt, spacing: )";
  output += FormatCompactNumber(line_spacing_em);
  output += R"(em))";
  return output;
}

auto BuildPageSetup(double margin_top_cm, double margin_bottom_cm,
                    double margin_left_cm, double margin_right_cm)
    -> std::string {
  std::string output = "#set page(margin: (top: ";
  output += FormatCompactNumber(margin_top_cm);
  output += "cm, bottom: ";
  output += FormatCompactNumber(margin_bottom_cm);
  output += "cm, left: ";
  output += FormatCompactNumber(margin_left_cm);
  output += "cm, right: ";
  output += FormatCompactNumber(margin_right_cm);
  output += "cm))";
  return output;
}

auto BuildTitleText(const std::string& category_title_font,
                    int category_title_font_size, const std::string& title_text)
    -> std::string {
  std::string output;
  output.reserve(category_title_font.size() + title_text.size() +
                 kTitleTextReservePadding);
  output += R"(#text(font: ")";
  output += category_title_font;
  output += R"(", size: )";
  output += std::to_string(category_title_font_size);
  output += R"(pt)[= )";
  output += title_text;
  output += "])";
  return output;
}

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

auto FormatProjectTree(const TtProjectTreeNodeV1* nodes, uint32_t node_count,
                       long long total_duration, int avg_days,
                       const std::string& category_title_font,
                       int category_title_font_size) -> std::string {
  auto strategy = std::make_unique<TypstFormattingStrategy>(
      category_title_font, category_title_font_size);
  reporting::ProjectTreeFormatter formatter(std::move(strategy));
  return formatter.FormatProjectTree(nodes, node_count, total_duration,
                                     avg_days);
}
// NOLINTEND(bugprone-easily-swappable-parameters)

}  // namespace TypUtils
