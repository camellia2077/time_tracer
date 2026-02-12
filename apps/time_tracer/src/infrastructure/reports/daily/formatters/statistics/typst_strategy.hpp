// infrastructure/reports/daily/formatters/statistics/typst_strategy.hpp
#ifndef REPORTS_DAILY_FORMATTERS_STATISTICS_TYPST_STRATEGY_H_
#define REPORTS_DAILY_FORMATTERS_STATISTICS_TYPST_STRATEGY_H_

#include <memory>
#include <string>
#include <vector>

#include "infrastructure/reports/daily/formatters/statistics/i_stat_strategy.hpp"
#include "infrastructure/reports/daily/formatters/typst/day_typ_config.hpp"

class TypstStrategy : public IStatStrategy {
 public:
  explicit TypstStrategy(const std::shared_ptr<DayTypConfig>& config);

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
  std::shared_ptr<DayTypConfig> config_;
};

#endif  // REPORTS_DAILY_FORMATTERS_STATISTICS_TYPST_STRATEGY_H_
