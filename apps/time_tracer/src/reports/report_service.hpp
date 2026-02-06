// reports/report_service.hpp
#ifndef REPORTS_REPORT_SERVICE_H_
#define REPORTS_REPORT_SERVICE_H_

#include <sqlite3.h>

#include <string>
#include <vector>

#include "common/config/app_config.hpp"
#include "reports/data/model/query_data_structs.hpp"
#include "reports/shared/types/report_format.hpp"

class ReportService {
 public:
  explicit ReportService(sqlite3* sqlite_db, const AppConfig& config);

  // --- Single Queries ---
  [[nodiscard]] auto RunDailyQuery(std::string_view date_str,
                                   ReportFormat format) const -> std::string;
  [[nodiscard]] auto RunPeriodQuery(int days, ReportFormat format) const
      -> std::string;
  [[nodiscard]] auto RunMonthlyQuery(std::string_view year_month_str,
                                     ReportFormat format) const -> std::string;
  [[nodiscard]] auto RunWeeklyQuery(std::string_view iso_week_str,
                                    ReportFormat format) const -> std::string;
  [[nodiscard]] auto RunYearlyQuery(std::string_view year_str,
                                    ReportFormat format) const -> std::string;

  // --- Bulk Export Queries ---
  [[nodiscard]] auto RunExportAllDailyReportsQuery(ReportFormat format) const
      -> FormattedGroupedReports;
  [[nodiscard]] auto RunExportAllMonthlyReportsQuery(ReportFormat format) const
      -> FormattedMonthlyReports;
  [[nodiscard]] auto RunExportAllPeriodReportsQuery(
      const std::vector<int>& days_list,
      ReportFormat format = ReportFormat::kMarkdown) const

      -> FormattedPeriodReports;
  [[nodiscard]] auto RunExportAllWeeklyReportsQuery(ReportFormat format) const
      -> FormattedWeeklyReports;
  [[nodiscard]] auto RunExportAllYearlyReportsQuery(ReportFormat format) const
      -> FormattedYearlyReports;

 private:
  sqlite3* db_;
  const AppConfig& app_config_;  // [ADDED] Store a reference to the config
};

#endif  // REPORTS_REPORT_SERVICE_H_
