// application/ports/i_report_export_writer.hpp
#ifndef APPLICATION_PORTS_I_REPORT_EXPORT_WRITER_H_
#define APPLICATION_PORTS_I_REPORT_EXPORT_WRITER_H_

#include <map>
#include <string>

#include "domain/reports/models/daily_report_data.hpp"
#include "domain/reports/models/period_report_models.hpp"
#include "domain/reports/types/report_types.hpp"

namespace tracer_core::application::ports {

class IReportExportWriter {
 public:
  virtual ~IReportExportWriter() = default;

  virtual auto ExportSingleDay(const std::string& date,
                               const DailyReportData& report,
                               ReportFormat format) -> void = 0;
  virtual auto ExportSingleMonth(const std::string& month,
                                 const MonthlyReportData& report,
                                 ReportFormat format) -> void = 0;
  virtual auto ExportSinglePeriod(int days, const PeriodReportData& report,
                                  ReportFormat format) -> void = 0;
  virtual auto ExportSingleWeek(const std::string& iso_week,
                                const WeeklyReportData& report,
                                ReportFormat format) -> void = 0;
  virtual auto ExportSingleYear(const std::string& year,
                                const YearlyReportData& report,
                                ReportFormat format) -> void = 0;

  virtual auto ExportAllDaily(
      const std::map<std::string, DailyReportData>& reports,
      ReportFormat format) -> void = 0;
  virtual auto ExportAllMonthly(
      const std::map<std::string, MonthlyReportData>& reports,
      ReportFormat format) -> void = 0;
  virtual auto ExportAllPeriod(const std::map<int, PeriodReportData>& reports,
                               ReportFormat format) -> void = 0;
  virtual auto ExportAllWeekly(
      const std::map<std::string, WeeklyReportData>& reports,
      ReportFormat format) -> void = 0;
  virtual auto ExportAllYearly(
      const std::map<std::string, YearlyReportData>& reports,
      ReportFormat format) -> void = 0;
};

}  // namespace tracer_core::application::ports

#endif  // APPLICATION_PORTS_I_REPORT_EXPORT_WRITER_H_
