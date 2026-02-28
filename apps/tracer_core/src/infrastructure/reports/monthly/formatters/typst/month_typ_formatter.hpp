// infrastructure/reports/monthly/formatters/typst/month_typ_formatter.hpp
#ifndef INFRASTRUCTURE_REPORTS_MONTHLY_FORMATTERS_TYPST_MONTH_TYP_FORMATTER_H_
#define INFRASTRUCTURE_REPORTS_MONTHLY_FORMATTERS_TYPST_MONTH_TYP_FORMATTER_H_

#include "domain/reports/models/period_report_models.hpp"
#include "infrastructure/reports/monthly/formatters/typst/month_typ_config.hpp"
#include "infrastructure/reports/shared/formatters/templates/base_typ_formatter.hpp"
#include "infrastructure/reports/shared/interfaces/formatter_c_abi_v2.hpp"

class MonthTypFormatter
    : public BaseTypFormatter<MonthlyReportData, MonthTypConfig> {
 public:
  explicit MonthTypFormatter(std::shared_ptr<MonthTypConfig> config);
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
  void FormatPageSetup(std::string& report_stream) const override;
};

#endif  // INFRASTRUCTURE_REPORTS_MONTHLY_FORMATTERS_TYPST_MONTH_TYP_FORMATTER_H_
