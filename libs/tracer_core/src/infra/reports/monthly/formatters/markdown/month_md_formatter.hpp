// infra/reports/monthly/formatters/markdown/month_md_formatter.hpp
#ifndef INFRASTRUCTURE_REPORTS_MONTHLY_FORMATTERS_MARKDOWN_MONTH_MD_FORMATTER_H_
#define INFRASTRUCTURE_REPORTS_MONTHLY_FORMATTERS_MARKDOWN_MONTH_MD_FORMATTER_H_

#include "domain/reports/models/period_report_models.hpp"
#include "infra/reports/monthly/formatters/markdown/month_md_config.hpp"
#include "infra/reports/shared/formatters/templates/base_md_formatter.hpp"

class MonthMdFormatter
    : public BaseMdFormatter<MonthlyReportData, MonthMdConfig> {
 public:
  explicit MonthMdFormatter(std::shared_ptr<MonthMdConfig> config);

 protected:
  [[nodiscard]] auto ValidateData(const MonthlyReportData& data) const
      -> std::string override;
  [[nodiscard]] auto IsEmptyData(const MonthlyReportData& data) const
      -> bool override;
  [[nodiscard]] auto GetAvgDays(const MonthlyReportData& data) const
      -> int override;
  [[nodiscard]] auto GetNoRecordsMsg() const -> std::string override;
  void FormatHeaderContent(std::string& report_stream,
                           const MonthlyReportData& data) const override;
};

#endif  // INFRASTRUCTURE_REPORTS_MONTHLY_FORMATTERS_MARKDOWN_MONTH_MD_FORMATTER_H_
