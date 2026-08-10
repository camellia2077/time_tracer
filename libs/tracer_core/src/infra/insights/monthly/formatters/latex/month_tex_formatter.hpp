// infra/insights/monthly/formatters/latex/month_tex_formatter.hpp
#ifndef INFRASTRUCTURE_INSIGHTS_MONTHLY_FORMATTERS_LATEX_MONTH_TEX_FORMATTER_H_
#define INFRASTRUCTURE_INSIGHTS_MONTHLY_FORMATTERS_LATEX_MONTH_TEX_FORMATTER_H_

#include "domain/insights/models/period_insights_models.hpp"
#include "infra/insights/monthly/formatters/latex/month_tex_config.hpp"
#include "infra/insights/shared/formatters/templates/base_tex_formatter.hpp"

class MonthTexFormatter
    : public BaseTexFormatter<MonthlyInsightsData, MonthTexConfig> {
 public:
  explicit MonthTexFormatter(std::shared_ptr<MonthTexConfig> config);

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
};

#endif  // INFRASTRUCTURE_INSIGHTS_MONTHLY_FORMATTERS_LATEX_MONTH_TEX_FORMATTER_H_
