// reports/monthly/formatters/typst/month_typ_formatter.hpp
#ifndef REPORTS_MONTHLY_FORMATTERS_TYPST_MONTH_TYP_FORMATTER_H_
#define REPORTS_MONTHLY_FORMATTERS_TYPST_MONTH_TYP_FORMATTER_H_

#include "reports/data/model/monthly_report_data.hpp"
#include "reports/monthly/formatters/typst/month_typ_config.hpp"
#include "reports/shared/formatters/templates/base_typ_formatter.hpp"

class MonthTypFormatter
    : public BaseTypFormatter<MonthlyReportData, MonthTypConfig> {
 public:
  explicit MonthTypFormatter(std::shared_ptr<MonthTypConfig> config);

 protected:
  auto ValidateData(const MonthlyReportData& data) const
      -> std::string override;
  auto IsEmptyData(const MonthlyReportData& data) const -> bool override;
  auto GetAvgDays(const MonthlyReportData& data) const -> int override;
  auto GetNoRecordsMsg() const -> std::string override;
  void FormatHeaderContent(std::stringstream& ss,
                           const MonthlyReportData& data) const override;
  void FormatPageSetup(std::stringstream& ss) const override;
};

#endif  // REPORTS_MONTHLY_FORMATTERS_TYPST_MONTH_TYP_FORMATTER_H_