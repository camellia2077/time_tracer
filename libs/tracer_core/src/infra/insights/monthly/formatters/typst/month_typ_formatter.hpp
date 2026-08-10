// infra/insights/monthly/formatters/typst/month_typ_formatter.hpp
#ifndef INFRASTRUCTURE_INSIGHTS_MONTHLY_FORMATTERS_TYPST_MONTH_TYP_FORMATTER_H_
#define INFRASTRUCTURE_INSIGHTS_MONTHLY_FORMATTERS_TYPST_MONTH_TYP_FORMATTER_H_

#include "domain/insights/models/period_insights_models.hpp"
#include "infra/insights/monthly/formatters/typst/month_typ_config.hpp"
#include "infra/insights/shared/formatters/templates/base_typ_formatter.hpp"

class MonthTypFormatter
    : public BaseTypFormatter<MonthlyInsightsData, MonthTypConfig> {
 public:
  explicit MonthTypFormatter(std::shared_ptr<MonthTypConfig> config);

 protected:
  [[nodiscard]] auto ValidateData(const MonthlyInsightsData& data) const
      -> std::string override;
  [[nodiscard]] auto IsEmptyData(const MonthlyInsightsData& data) const
      -> bool override;
  [[nodiscard]] auto GetAvgDays(const MonthlyInsightsData& data) const
      -> int override;
  [[nodiscard]] auto GetNoRecordsMsg() const -> std::string override;
  void FormatHeaderContent(std::string& insights_stream,
                           const MonthlyInsightsData& data) const override;
  void FormatPageSetup(std::string& insights_stream) const override;
};

#endif  // INFRASTRUCTURE_INSIGHTS_MONTHLY_FORMATTERS_TYPST_MONTH_TYP_FORMATTER_H_
