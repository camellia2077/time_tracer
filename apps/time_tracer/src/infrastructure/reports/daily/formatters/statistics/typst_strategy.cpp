// infrastructure/reports/daily/formatters/statistics/typst_strategy.cpp
#include "infrastructure/reports/daily/formatters/statistics/typst_strategy.hpp"

namespace {
constexpr size_t kHeaderReservePadding = 96;
constexpr size_t kMainItemPadding = 8;
constexpr size_t kSubItemPadding = 10;
}  // namespace

TypstStrategy::TypstStrategy(const std::shared_ptr<DayTypConfig>& config)
    : config_(config) {}

auto TypstStrategy::FormatHeader(const std::string& title) const
    -> std::string {
  std::string header;
  header.reserve(title.size() + kHeaderReservePadding);
  header += "#let statistic_font_size = ";
  header += std::to_string(config_->GetStatisticFontSize());
  header += "pt\n";
  header += "#let statistic_title_font_size = ";
  header += std::to_string(config_->GetStatisticTitleFontSize());
  header += "pt\n";
  header += "#set text(size: statistic_font_size)\n";
  header += "#text(size: statistic_title_font_size)[= ";
  header += title;
  header += "]\n\n";
  return header;
}

auto TypstStrategy::FormatMainItem(const std::string& label,
                                   const std::string& value) const
    -> std::string {
  std::string line;
  line.reserve(label.size() + value.size() + kMainItemPadding);
  line += "- *";
  line += label;
  line += "*: ";
  line += value;
  return line;
}

auto TypstStrategy::FormatSubItem(const std::string& label,
                                  const std::string& value) const
    -> std::string {
  std::string line;
  line.reserve(label.size() + value.size() + kSubItemPadding);
  line += "  - *";
  line += label;
  line += "*: ";
  line += value;
  return line;
}

auto TypstStrategy::BuildOutput(const std::vector<std::string>& lines) const
    -> std::string {
  std::string result;
  for (const auto& line : lines) {
    result += line;
    result += "\n";
  }
  return result;
}
