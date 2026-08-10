// infra/insights/daily/formatters/markdown/day_md_formatter.hpp
#ifndef INFRASTRUCTURE_INSIGHTS_DAILY_FORMATTERS_MARKDOWN_DAY_MD_FORMATTER_H_
#define INFRASTRUCTURE_INSIGHTS_DAILY_FORMATTERS_MARKDOWN_DAY_MD_FORMATTER_H_

#include "domain/insights/models/daily_insights_data.hpp"
#include "infra/insights/daily/formatters/markdown/day_md_config.hpp"
#include "infra/insights/shared/formatters/templates/base_md_formatter.hpp"

class DayMdFormatter : public BaseMdFormatter<DailyInsightsData, DayMdConfig> {
 public:
  explicit DayMdFormatter(std::shared_ptr<DayMdConfig> config);

 protected:
  // --- 实现基类钩子 ---
  [[nodiscard]] auto IsEmptyData(const DailyInsightsData& data) const
      -> bool override;
  [[nodiscard]] auto GetAvgDays(const DailyInsightsData& data) const
      -> int override;
  [[nodiscard]] auto GetNoRecordsMsg() const -> std::string override;

  void FormatHeaderContent(std::string& insights_stream,
                           const DailyInsightsData& data) const override;
  void FormatExtraContent(std::string& insights_stream,
                          const DailyInsightsData& data) const override;

 private:
  // 原本的私有辅助函数现在变成了钩子函数的具体实现，或者被内联
  // 原本的私有辅助函数现在变成了钩子函数的具体实现，或者被内联
  void DisplayDetailedActivities(std::string& insights_stream,
                                 const DailyInsightsData& data) const;
};

#endif  // INFRASTRUCTURE_INSIGHTS_DAILY_FORMATTERS_MARKDOWN_DAY_MD_FORMATTER_H_
