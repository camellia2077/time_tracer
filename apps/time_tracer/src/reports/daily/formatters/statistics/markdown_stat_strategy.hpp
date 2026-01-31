// reports/daily/formatters/statistics/markdown_stat_strategy.hpp
#ifndef REPORTS_DAILY_FORMATTERS_STATISTICS_MARKDOWN_STAT_STRATEGY_H_
#define REPORTS_DAILY_FORMATTERS_STATISTICS_MARKDOWN_STAT_STRATEGY_H_

#include <format>

#include "i_stat_strategy.hpp"

class MarkdownStatStrategy : public IStatStrategy {
 public:
  std::string format_header(const std::string& title) const override {
    return std::format("\n## {}\n\n", title);
  }

  std::string format_main_item(const std::string& label,
                               const std::string& value) const override {
    return std::format("- **{0}**: {1}", label, value);
  }

  std::string format_sub_item(const std::string& label,
                              const std::string& value) const override {
    return std::format("  - **{0}**: {1}", label, value);
  }

  std::string build_output(
      const std::vector<std::string>& lines) const override {
    std::string result;
    for (const auto& line : lines) {
      result += line + "\n";
    }
    return result;
  }
};

#endif  // REPORTS_DAILY_FORMATTERS_STATISTICS_MARKDOWN_STAT_STRATEGY_H_