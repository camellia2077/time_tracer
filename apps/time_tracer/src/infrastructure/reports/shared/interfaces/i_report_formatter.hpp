// infrastructure/reports/shared/interfaces/i_report_formatter.hpp
#ifndef REPORTS_SHARED_INTERFACES_I_REPORT_FORMATTER_H_
#define REPORTS_SHARED_INTERFACES_I_REPORT_FORMATTER_H_

#include <string>

#include "domain/reports/models/daily_report_data.hpp"
#include "domain/reports/models/monthly_report_data.hpp"
#include "domain/reports/models/period_report_data.hpp"
#include "domain/reports/models/range_report_data.hpp"
#include "domain/reports/models/weekly_report_data.hpp"
#include "domain/reports/models/yearly_report_data.hpp"

template <typename ReportDataType>
class IReportFormatter {
 public:
  virtual ~IReportFormatter() = default;
  [[nodiscard]] virtual auto FormatReport(const ReportDataType& data) const
      -> std::string = 0;
};

#endif  // REPORTS_SHARED_INTERFACES_I_REPORT_FORMATTER_H_
