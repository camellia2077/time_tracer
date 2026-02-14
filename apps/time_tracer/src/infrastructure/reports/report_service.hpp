// infrastructure/reports/report_service.hpp
#ifndef INFRASTRUCTURE_REPORTS_REPORT_SERVICE_H_
#define INFRASTRUCTURE_REPORTS_REPORT_SERVICE_H_

#include <sqlite3.h>

#include <map>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "application/interfaces/i_report_query_service.hpp"
#include "application/ports/i_platform_clock.hpp"
#include "domain/reports/models/period_report_models.hpp"
#include "infrastructure/config/models/report_catalog.hpp"
#include "infrastructure/reports/shared/interfaces/i_report_formatter.hpp"

class ReportService : public IReportQueryService {
 public:
  ReportService(sqlite3* sqlite_db, const ReportCatalog& catalog,
                std::shared_ptr<time_tracer::application::ports::IPlatformClock>
                    platform_clock);
  ~ReportService() override = default;

  // --- Single Queries ---
  [[nodiscard]] auto RunDailyQuery(std::string_view date_str,
                                   ReportFormat format) const
      -> std::string override;
  [[nodiscard]] auto RunPeriodQuery(int days, ReportFormat format) const
      -> std::string override;
  [[nodiscard]] auto RunMonthlyQuery(std::string_view year_month_str,
                                     ReportFormat format) const
      -> std::string override;
  [[nodiscard]] auto RunWeeklyQuery(std::string_view iso_week_str,
                                    ReportFormat format) const
      -> std::string override;
  [[nodiscard]] auto RunYearlyQuery(std::string_view year_str,
                                    ReportFormat format) const
      -> std::string override;

  // --- Bulk Export Queries ---
  [[nodiscard]] auto RunExportAllDailyReportsQuery(ReportFormat format) const
      -> FormattedGroupedReports override;
  [[nodiscard]] auto RunExportAllMonthlyReportsQuery(ReportFormat format) const
      -> FormattedMonthlyReports override;
  [[nodiscard]] auto RunExportAllPeriodReportsQuery(
      const std::vector<int>& days_list, ReportFormat format) const
      -> FormattedPeriodReports override;
  [[nodiscard]] auto RunExportAllWeeklyReportsQuery(ReportFormat format) const
      -> FormattedWeeklyReports override;
  [[nodiscard]] auto RunExportAllYearlyReportsQuery(ReportFormat format) const
      -> FormattedYearlyReports override;

 private:
  [[nodiscard]] auto GetOrCreatePeriodFormatter(ReportFormat format) const
      -> IReportFormatter<PeriodReportData>&;

  sqlite3* db_;
  const ReportCatalog& report_catalog_;
  std::shared_ptr<time_tracer::application::ports::IPlatformClock>
      platform_clock_;
  mutable std::map<ReportFormat,
                   std::unique_ptr<IReportFormatter<PeriodReportData>>>
      period_formatter_cache_;
};

#endif  // INFRASTRUCTURE_REPORTS_REPORT_SERVICE_H_
