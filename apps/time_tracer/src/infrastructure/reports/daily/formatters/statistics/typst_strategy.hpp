// infrastructure/reports/daily/formatters/statistics/typst_strategy.hpp
#ifndef REPORTS_DAILY_FORMATTERS_STATISTICS_TYPST_STRATEGY_H_
#define REPORTS_DAILY_FORMATTERS_STATISTICS_TYPST_STRATEGY_H_

#include <format>
#include <vector>

#include "infrastructure/reports/daily/formatters/statistics/i_stat_strategy.hpp"
#include "infrastructure/reports/daily/formatters/typst/day_typ_config.hpp"

class TypstStrategy : public IStatStrategy {
 public:
  explicit TypstStrategy(const std::shared_ptr<DayTypConfig>& config)
      : config_(config) {}

  [[nodiscard]] auto FormatHeader(const std::string& title) const
      -> std::string override {
    std::string header;
    header += std::format("#let statistic_font_size = {}pt\n",
                          config_->GetStatisticFontSize());
    header += std::format("#let statistic_title_font_size = {}pt\n",
                          config_->GetStatisticTitleFontSize());
    header += "#set text(size: statistic_font_size)\n";
    // 注意必须是 = {0}而不是={0}，不然会没法正确渲染标题
    header +=
        std::format("#text(size: statistic_title_font_size)[= {0}]\n\n", title);

    return header;
  }

  [[nodiscard]] auto FormatMainItem(const std::string& label,
                                    const std::string& value) const
      -> std::string override {
    return std::format("- *{0}*: {1}", label, value);
  }

  [[nodiscard]] auto FormatSubItem(const std::string& label,
                                   const std::string& value) const
      -> std::string override {
    return std::format("  - *{0}*: {1}", label, value);
  }

  [[nodiscard]] auto BuildOutput(const std::vector<std::string>& lines) const
      -> std::string override {
    std::string result;
    for (const auto& line : lines) {
      result += line + "\n";
    }
    return result;
  }

 private:
  std::shared_ptr<DayTypConfig> config_;
};
#endif  // REPORTS_DAILY_FORMATTERS_STATISTICS_TYPST_STRATEGY_H_