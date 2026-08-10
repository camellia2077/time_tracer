// infra/insights/monthly/formatters/markdown/month_md_formatter.hpp
#ifndef INFRASTRUCTURE_INSIGHTS_MONTHLY_FORMATTERS_MARKDOWN_MONTH_MD_FORMATTER_H_
#define INFRASTRUCTURE_INSIGHTS_MONTHLY_FORMATTERS_MARKDOWN_MONTH_MD_FORMATTER_H_

#include "domain/insights/models/period_insights_models.hpp"
#include "infra/insights/monthly/formatters/markdown/month_md_config.hpp"
#include "infra/insights/shared/formatters/templates/base_md_formatter.hpp"

class MonthMdFormatter
    : public BaseMdFormatter<MonthlyInsightsData, MonthMdConfig> {
 public:
  explicit MonthMdFormatter(std::shared_ptr<MonthMdConfig> config);

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

#endif  // INFRASTRUCTURE_INSIGHTS_MONTHLY_FORMATTERS_MARKDOWN_MONTH_MD_FORMATTER_H_
