// infra/insights/daily/formatters/statistics/latex_strategy.hpp
#ifndef INFRASTRUCTURE_INSIGHTS_DAILY_FORMATTERS_STATISTICS_LATEX_STRATEGY_H_
#define INFRASTRUCTURE_INSIGHTS_DAILY_FORMATTERS_STATISTICS_LATEX_STRATEGY_H_

#include <memory>
#include <string>
#include <vector>

#include "infra/insights/daily/formatters/latex/daily_tex_formatter_config.hpp"
#include "infra/insights/daily/formatters/statistics/i_stat_strategy.hpp"

class LatexStrategy : public IStatStrategy {
 public:
  explicit LatexStrategy(
      const std::shared_ptr<DailyTexFormatterConfig>& config);

  [[nodiscard]] auto FormatHeader(const std::string& title) const
      -> std::string override;
  [[nodiscard]] auto FormatMainItem(const std::string& label,
                                    const std::string& value) const
      -> std::string override;
  [[nodiscard]] auto FormatSubItem(const std::string& label,
                                   const std::string& value) const
      -> std::string override;
  [[nodiscard]] auto BuildOutput(const std::vector<std::string>& lines) const
      -> std::string override;

 private:
  std::shared_ptr<DailyTexFormatterConfig> config_;
};

#endif  // INFRASTRUCTURE_INSIGHTS_DAILY_FORMATTERS_STATISTICS_LATEX_STRATEGY_H_
