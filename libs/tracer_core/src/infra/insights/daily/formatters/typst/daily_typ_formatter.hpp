// infra/insights/daily/formatters/typst/daily_typ_formatter.hpp
#ifndef INFRASTRUCTURE_INSIGHTS_DAILY_FORMATTERS_TYPST_DAILY_TYP_FORMATTER_H_
#define INFRASTRUCTURE_INSIGHTS_DAILY_FORMATTERS_TYPST_DAILY_TYP_FORMATTER_H_

#include "domain/insights/models/daily_insights_data.hpp"
#include "infra/insights/daily/formatters/typst/daily_typ_formatter_config.hpp"
#include "infra/insights/shared/formatters/templates/base_typ_formatter.hpp"

class DailyTypFormatter
    : public BaseTypFormatter<DailyInsightsData, DailyTypFormatterConfig> {
 public:
  explicit DailyTypFormatter(std::shared_ptr<DailyTypFormatterConfig> config);

 protected:
  // --- 实现基类钩子 ---
  // --- 实现基类钩子 ---
  [[nodiscard]] auto IsEmptyData(const DailyInsightsData& data) const
      -> bool override;
  [[nodiscard]] auto GetAvgDays(const DailyInsightsData& data) const
      -> int override;

  void FormatHeaderContent(std::string& insights_stream,
                           const DailyInsightsData& data) const override;
  void FormatExtraContent(std::string& insights_stream,
                          const DailyInsightsData& data) const override;

  // 适配接口
  [[nodiscard]] auto GetNoRecordsMsg() const -> std::string override;
};

#endif  // INFRASTRUCTURE_INSIGHTS_DAILY_FORMATTERS_TYPST_DAILY_TYP_FORMATTER_H_
