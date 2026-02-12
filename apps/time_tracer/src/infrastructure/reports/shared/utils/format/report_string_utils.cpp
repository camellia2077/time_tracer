// infrastructure/reports/shared/utils/format/report_string_utils.cpp
#include "infrastructure/reports/shared/utils/format/report_string_utils.hpp"

namespace {
constexpr std::size_t kPercentageReserve = 24U;
constexpr int kFractionThreshold = 10;
constexpr std::size_t kResultSuffixReserve = 16U;
}  // namespace

auto ReplaceAll(std::string str, const std::string& from,
                const std::string& replacement) -> std::string {
  size_t start_pos = 0;
  while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
    str.replace(start_pos, from.length(), replacement);
    start_pos += replacement.length();
  }
  return str;
}

auto FormatMultilineForList(const std::string& text, int indent_spaces,
                            const std::string& line_suffix) -> std::string {
  if (text.empty()) {
    return "";
  }

  std::string indent(indent_spaces, ' ');
  // 将 "\n" 替换为 "后缀 + \n + 缩进"
  std::string replacement = line_suffix + "\n" + indent;

  return ReplaceAll(text, "\n", replacement);
}

auto FormatTitleTemplate(std::string title_template,
                         const RangeReportData& data) -> std::string {
  const std::string kRequestedDays =
      (data.requested_days > 0) ? std::to_string(data.requested_days) : "";

  title_template =
      ReplaceAll(title_template, "{range_label}", data.range_label);
  title_template = ReplaceAll(title_template, "{start_date}", data.start_date);
  title_template = ReplaceAll(title_template, "{end_date}", data.end_date);
  title_template =
      ReplaceAll(title_template, "{requested_days}", kRequestedDays);

  // Backward-compatible aliases
  title_template = ReplaceAll(title_template, "{year_month}", data.range_label);
  title_template =
      ReplaceAll(title_template, "{days_to_query}", kRequestedDays);

  return title_template;
}

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
auto FormatCountWithPercentage(int count, int total_count,
                               const std::string& percent_suffix)
    -> std::string {
  if (total_count <= 0) {
    return std::to_string(count);
  }

  const auto kDenominator = static_cast<long long>(total_count);
  const auto kScaled =
      static_cast<long long>(count) * static_cast<long long>(10000);
  const auto kRounded = (kScaled >= 0)
                            ? ((kScaled + (kDenominator / 2)) / kDenominator)
                            : ((kScaled - (kDenominator / 2)) / kDenominator);

  long long abs_value = (kRounded < 0) ? -kRounded : kRounded;
  const auto kWhole = abs_value / 100;
  const auto kFraction = abs_value % 100;

  std::string percentage;
  percentage.reserve(kPercentageReserve);
  if (kRounded < 0) {
    percentage.push_back('-');
  }
  percentage += std::to_string(kWhole);
  percentage.push_back('.');
  if (kFraction < kFractionThreshold) {
    percentage.push_back('0');
  }
  percentage += std::to_string(kFraction);
  percentage += percent_suffix;

  std::string result;
  result.reserve(percentage.size() + kResultSuffixReserve);
  result += std::to_string(count);
  result += " (";
  result += percentage;
  result.push_back(')');
  return result;
}
