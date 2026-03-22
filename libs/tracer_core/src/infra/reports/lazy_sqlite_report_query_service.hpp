#ifndef INFRASTRUCTURE_REPORTS_LAZY_SQLITE_REPORT_QUERY_SERVICE_H_
#define INFRASTRUCTURE_REPORTS_LAZY_SQLITE_REPORT_QUERY_SERVICE_H_

#include <filesystem>
#include <memory>

#include "application/interfaces/i_report_query_service.hpp"
#include "application/ports/i_platform_clock.hpp"
#include "infra/config/models/report_catalog.hpp"

namespace tracer::core::infrastructure::reports {
class LazySqliteReportQueryService final : public IReportQueryService {
 public:
  LazySqliteReportQueryService(
      std::filesystem::path db_path,
      std::shared_ptr<ReportCatalog> report_catalog,
      std::shared_ptr<tracer_core::application::ports::IPlatformClock>
          platform_clock);

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
  std::filesystem::path db_path_;
  std::shared_ptr<ReportCatalog> report_catalog_;
  std::shared_ptr<tracer_core::application::ports::IPlatformClock>
      platform_clock_;
};

}  // namespace tracer::core::infrastructure::reports

namespace infrastructure::reports {

using tracer::core::infrastructure::reports::LazySqliteReportQueryService;

}  // namespace infrastructure::reports

#endif  // INFRASTRUCTURE_REPORTS_LAZY_SQLITE_REPORT_QUERY_SERVICE_H_
