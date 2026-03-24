// infra/reporting/report_dto_formatter.hpp
#ifndef INFRASTRUCTURE_REPORTS_REPORT_DTO_FORMATTER_H_
#define INFRASTRUCTURE_REPORTS_REPORT_DTO_FORMATTER_H_

#include <map>
#include <memory>

#include "application/ports/reporting/i_report_dto_formatter.hpp"
#include "infra/config/models/report_catalog.hpp"
#include "infra/reporting/shared/interfaces/i_report_formatter.hpp"

namespace tracer::core::infrastructure::reports {
class ReportDtoFormatter final
    : public tracer_core::application::ports::IReportDtoFormatter {
 public:
  explicit ReportDtoFormatter(const ReportCatalog& report_catalog);

  auto FormatDaily(const DailyReportData& report, ReportFormat format)
      -> std::string override;
  auto FormatMonthly(const MonthlyReportData& report, ReportFormat format)
      -> std::string override;
  auto FormatPeriod(const PeriodReportData& report, ReportFormat format)
      -> std::string override;
  auto FormatWeekly(const WeeklyReportData& report, ReportFormat format)
      -> std::string override;
  auto FormatYearly(const YearlyReportData& report, ReportFormat format)
      -> std::string override;

 private:
  template <typename ReportDataType>
  auto FormatWithCache(
      const ReportDataType& report, ReportFormat format,
      std::map<ReportFormat, std::unique_ptr<IReportFormatter<ReportDataType>>>&
          cache) -> std::string;

  const ReportCatalog& report_catalog_;
  std::map<ReportFormat, std::unique_ptr<IReportFormatter<DailyReportData>>>
      daily_cache_;
  std::map<ReportFormat, std::unique_ptr<IReportFormatter<MonthlyReportData>>>
      monthly_cache_;
  std::map<ReportFormat, std::unique_ptr<IReportFormatter<PeriodReportData>>>
      period_cache_;
  std::map<ReportFormat, std::unique_ptr<IReportFormatter<WeeklyReportData>>>
      weekly_cache_;
  std::map<ReportFormat, std::unique_ptr<IReportFormatter<YearlyReportData>>>
      yearly_cache_;
};

}  // namespace tracer::core::infrastructure::reports

namespace infrastructure::reports {

using tracer::core::infrastructure::reports::ReportDtoFormatter;

}  // namespace infrastructure::reports

#endif  // INFRASTRUCTURE_REPORTS_REPORT_DTO_FORMATTER_H_
