// infrastructure/reports/daily/formatters/statistics/markdown_stat_strategy.hpp
#ifndef INFRASTRUCTURE_REPORTS_DAILY_FORMATTERS_STATISTICS_MARKDOWN_STAT_STRATEGY_H_
#define INFRASTRUCTURE_REPORTS_DAILY_FORMATTERS_STATISTICS_MARKDOWN_STAT_STRATEGY_H_

#include "infrastructure/reports/daily/formatters/statistics/i_stat_strategy.hpp"

class MarkdownStatStrategy : public IStatStrategy {
 public:
  [[nodiscard]] auto FormatHeader(const std::string& title) const
      -> std::string override {
    return "\n## " + title + "\n\n";
  }

  [[nodiscard]] auto FormatMainItem(const std::string& label,
                                    const std::string& value) const
      -> std::string override {
    return "- **" + label + "**: " + value;
  }

  [[nodiscard]] auto FormatSubItem(const std::string& label,
                                   const std::string& value) const
      -> std::string override {
    return "  - **" + label + "**: " + value;
  }

  [[nodiscard]] auto BuildOutput(const std::vector<std::string>& lines) const
      -> std::string override {
    std::string result;
    for (const auto& line : lines) {
      result += line + "\n";
    }
    return result;
  }
};

#endif  // INFRASTRUCTURE_REPORTS_DAILY_FORMATTERS_STATISTICS_MARKDOWN_STAT_STRATEGY_H_
