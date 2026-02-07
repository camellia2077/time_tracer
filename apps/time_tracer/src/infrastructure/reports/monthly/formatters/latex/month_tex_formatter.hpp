// infrastructure/reports/monthly/formatters/latex/month_tex_formatter.hpp
#ifndef REPORTS_MONTHLY_FORMATTERS_LATEX_MONTH_TEX_FORMATTER_H_
#define REPORTS_MONTHLY_FORMATTERS_LATEX_MONTH_TEX_FORMATTER_H_

#include "domain/reports/models/monthly_report_data.hpp"
#include "infrastructure/reports/monthly/formatters/latex/month_tex_config.hpp"
#include "infrastructure/reports/shared/formatters/templates/base_tex_formatter.hpp"

class MonthTexFormatter
    : public BaseTexFormatter<MonthlyReportData, MonthTexConfig> {
 public:
  explicit MonthTexFormatter(std::shared_ptr<MonthTexConfig> config);

 protected:
  auto ValidateData(const MonthlyReportData& data) const
      -> std::string override;
  auto IsEmptyData(const MonthlyReportData& data) const -> bool override;
  auto GetAvgDays(const MonthlyReportData& data) const -> int override;
  auto GetNoRecordsMsg() const -> std::string override;
  void FormatHeaderContent(std::stringstream& report_stream,
                           const MonthlyReportData& data) const override;
};

#endif  // REPORTS_MONTHLY_FORMATTERS_LATEX_MONTH_TEX_FORMATTER_H_
