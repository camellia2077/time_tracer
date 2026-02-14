// infrastructure/reports/monthly/formatters/markdown/month_md_formatter.hpp
#ifndef INFRASTRUCTURE_REPORTS_MONTHLY_FORMATTERS_MARKDOWN_MONTH_MD_FORMATTER_H_
#define INFRASTRUCTURE_REPORTS_MONTHLY_FORMATTERS_MARKDOWN_MONTH_MD_FORMATTER_H_

#include "domain/reports/models/period_report_models.hpp"
#include "infrastructure/reports/monthly/formatters/markdown/month_md_config.hpp"
#include "infrastructure/reports/shared/formatters/templates/base_md_formatter.hpp"
#include "infrastructure/reports/shared/interfaces/formatter_c_abi_v2.hpp"

class MonthMdFormatter
    : public BaseMdFormatter<MonthlyReportData, MonthMdConfig> {
 public:
  explicit MonthMdFormatter(std::shared_ptr<MonthMdConfig> config);
  [[nodiscard]] auto FormatReportFromView(
      const TtRangeReportDataV1& data_view) const -> std::string;

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
