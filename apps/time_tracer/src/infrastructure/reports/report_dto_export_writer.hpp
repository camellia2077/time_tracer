// infrastructure/reports/report_dto_export_writer.hpp
#ifndef INFRASTRUCTURE_REPORTS_REPORT_DTO_EXPORT_WRITER_H_
#define INFRASTRUCTURE_REPORTS_REPORT_DTO_EXPORT_WRITER_H_

#include <map>
#include <memory>
#include <string>

#include "application/interfaces/i_report_exporter.hpp"
#include "application/ports/i_report_dto_formatter.hpp"
#include "application/ports/i_report_export_writer.hpp"

namespace infrastructure::reports {

class ReportDtoExportWriter final
    : public time_tracer::application::ports::IReportExportWriter {
 public:
  ReportDtoExportWriter(
      std::shared_ptr<time_tracer::application::ports::IReportDtoFormatter>
          formatter,
      std::shared_ptr<IReportExporter> exporter);

  auto ExportSingleDay(const std::string& date, const DailyReportData& report,
                       ReportFormat format) -> void override;
  auto ExportSingleMonth(const std::string& month,
                         const MonthlyReportData& report, ReportFormat format)
      -> void override;
  auto ExportSinglePeriod(int days, const PeriodReportData& report,
                          ReportFormat format) -> void override;
  auto ExportSingleWeek(const std::string& iso_week,
                        const WeeklyReportData& report, ReportFormat format)
      -> void override;
  auto ExportSingleYear(const std::string& year, const YearlyReportData& report,
                        ReportFormat format) -> void override;

  auto ExportAllDaily(const std::map<std::string, DailyReportData>& reports,
                      ReportFormat format) -> void override;
  auto ExportAllMonthly(const std::map<std::string, MonthlyReportData>& reports,
                        ReportFormat format) -> void override;
  auto ExportAllPeriod(const std::map<int, PeriodReportData>& reports,
                       ReportFormat format) -> void override;
  auto ExportAllWeekly(const std::map<std::string, WeeklyReportData>& reports,
                       ReportFormat format) -> void override;
  auto ExportAllYearly(const std::map<std::string, YearlyReportData>& reports,
                       ReportFormat format) -> void override;

 private:
  std::shared_ptr<time_tracer::application::ports::IReportDtoFormatter>
      formatter_;
  std::shared_ptr<IReportExporter> exporter_;
};

}  // namespace infrastructure::reports

#endif  // INFRASTRUCTURE_REPORTS_REPORT_DTO_EXPORT_WRITER_H_
