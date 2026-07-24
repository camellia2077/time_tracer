// application/ports/reporting/i_report_dto_formatter.hpp
#ifndef APPLICATION_PORTS_I_REPORT_DTO_FORMATTER_H_
#define APPLICATION_PORTS_I_REPORT_DTO_FORMATTER_H_

#include <string>
#include <string_view>

#include "domain/reports/models/daily_report_data.hpp"
#include "domain/reports/models/period_report_models.hpp"
#include "domain/reports/types/report_types.hpp"

namespace tracer_core::application::ports {

class IReportDtoFormatter {
 public:
  virtual ~IReportDtoFormatter() = default;

  virtual auto FormatDaily(const DailyReportData& report, ReportFormat format)
      -> std::string = 0;
  virtual auto FormatMonthly(const MonthlyReportData& report,
                             ReportFormat format) -> std::string = 0;
  virtual auto FormatPeriod(const PeriodReportData& report, ReportFormat format)
      -> std::string = 0;
  virtual auto FormatWeekly(const WeeklyReportData& report, ReportFormat format)
      -> std::string = 0;
  virtual auto FormatYearly(const YearlyReportData& report, ReportFormat format)
      -> std::string = 0;

  virtual auto FormatDailyLocalized(const DailyReportData& report,
                                    ReportFormat format,
                                    std::string_view locale) -> std::string {
    return FormatDaily(report, format);
  }
  virtual auto FormatMonthlyLocalized(const MonthlyReportData& report,
                                      ReportFormat format,
                                      std::string_view locale) -> std::string {
    return FormatMonthly(report, format);
  }
  virtual auto FormatPeriodLocalized(const PeriodReportData& report,
                                     ReportFormat format,
                                     std::string_view locale) -> std::string {
    return FormatPeriod(report, format);
  }
  virtual auto FormatWeeklyLocalized(const WeeklyReportData& report,
                                     ReportFormat format,
                                     std::string_view locale) -> std::string {
    return FormatWeekly(report, format);
  }
  virtual auto FormatYearlyLocalized(const YearlyReportData& report,
                                     ReportFormat format,
                                     std::string_view locale) -> std::string {
    return FormatYearly(report, format);
  }
};

}  // namespace tracer_core::application::ports

#endif  // APPLICATION_PORTS_I_REPORT_DTO_FORMATTER_H_
