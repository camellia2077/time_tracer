// infrastructure/reports/monthly/formatters/markdown/month_md_formatter.hpp
#ifndef REPORTS_MONTHLY_FORMATTERS_MARKDOWN_MONTH_MD_FORMATTER_H_
#define REPORTS_MONTHLY_FORMATTERS_MARKDOWN_MONTH_MD_FORMATTER_H_

#include "domain/reports/models/monthly_report_data.hpp"
#include "infrastructure/reports/monthly/formatters/markdown/month_md_config.hpp"
#include "infrastructure/reports/shared/formatters/templates/base_md_formatter.hpp"

class MonthMdFormatter
    : public BaseMdFormatter<MonthlyReportData, MonthMdConfig> {
 public:
  explicit MonthMdFormatter(std::shared_ptr<MonthMdConfig> config);

 protected:
  auto ValidateData(const MonthlyReportData& data) const
      -> std::string override;
  auto IsEmptyData(const MonthlyReportData& data) const -> bool override;
  auto GetAvgDays(const MonthlyReportData& data) const -> int override;
  auto GetNoRecordsMsg() const -> std::string override;
  void FormatHeaderContent(std::stringstream& report_stream,
                           const MonthlyReportData& data) const override;
};

#endif  // REPORTS_MONTHLY_FORMATTERS_MARKDOWN_MONTH_MD_FORMATTER_H_
