// infra/insights/daily/formatters/latex/day_tex_formatter.hpp
#ifndef INFRASTRUCTURE_INSIGHTS_DAILY_FORMATTERS_LATEX_DAY_TEX_FORMATTER_H_
#define INFRASTRUCTURE_INSIGHTS_DAILY_FORMATTERS_LATEX_DAY_TEX_FORMATTER_H_

#include "domain/insights/models/daily_insights_data.hpp"
#include "infra/insights/daily/formatters/latex/day_tex_config.hpp"
#include "infra/insights/shared/formatters/templates/base_tex_formatter.hpp"

class DayTexFormatter : public BaseTexFormatter<DailyInsightsData, DayTexConfig> {
 public:
  explicit DayTexFormatter(std::shared_ptr<DayTexConfig> config);

 protected:
  // --- 实现基类钩子 ---
  [[nodiscard]] auto IsEmptyData(const DailyInsightsData& data) const
      -> bool override;
  [[nodiscard]] auto GetAvgDays(const DailyInsightsData& data) const
      -> int override;

  void FormatHeaderContent(std::string& insights_stream,
                           const DailyInsightsData& data) const override;
  void FormatExtraContent(std::string& insights_stream,
                          const DailyInsightsData& data) const override;

  [[nodiscard]] auto GetKeywordColors() const
      -> std::map<std::string, std::string> override;

  // [新增] 实现基类定义的纯虚函数
  [[nodiscard]] auto GetNoRecordsMsg() const -> std::string override;
};

#endif  // INFRASTRUCTURE_INSIGHTS_DAILY_FORMATTERS_LATEX_DAY_TEX_FORMATTER_H_
