// infrastructure/reports/daily/formatters/statistics/latex_strategy.hpp
#ifndef REPORTS_DAILY_FORMATTERS_STATISTICS_LATEX_STRATEGY_H_
#define REPORTS_DAILY_FORMATTERS_STATISTICS_LATEX_STRATEGY_H_

#include <memory>
#include <string>
#include <vector>

#include "infrastructure/reports/daily/formatters/latex/day_tex_config.hpp"
#include "infrastructure/reports/daily/formatters/statistics/i_stat_strategy.hpp"

class LatexStrategy : public IStatStrategy {
 public:
  explicit LatexStrategy(const std::shared_ptr<DayTexConfig>& config);

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
  std::shared_ptr<DayTexConfig> config_;
};

#endif  // REPORTS_DAILY_FORMATTERS_STATISTICS_LATEX_STRATEGY_H_
