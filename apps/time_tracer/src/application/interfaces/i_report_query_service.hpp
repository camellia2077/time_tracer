// application/interfaces/i_report_query_service.hpp
#ifndef APPLICATION_INTERFACES_I_REPORT_QUERY_SERVICE_H_
#define APPLICATION_INTERFACES_I_REPORT_QUERY_SERVICE_H_

#include <string>
#include <string_view>
#include <vector>

#include "domain/reports/models/query_data_structs.hpp"
#include "domain/reports/types/report_format.hpp"

class IReportQueryService {
 public:
  virtual ~IReportQueryService() = default;

  // --- Single Queries ---
  [[nodiscard]] virtual auto RunDailyQuery(std::string_view date_str,
                                           ReportFormat format) const
      -> std::string = 0;
  [[nodiscard]] virtual auto RunPeriodQuery(int days, ReportFormat format) const
      -> std::string = 0;
  [[nodiscard]] virtual auto RunMonthlyQuery(std::string_view year_month_str,
                                             ReportFormat format) const
      -> std::string = 0;
  [[nodiscard]] virtual auto RunWeeklyQuery(std::string_view iso_week_str,
                                            ReportFormat format) const
      -> std::string = 0;
  [[nodiscard]] virtual auto RunYearlyQuery(std::string_view year_str,
                                            ReportFormat format) const
      -> std::string = 0;

  // --- Bulk Export Queries ---
  [[nodiscard]] virtual auto RunExportAllDailyReportsQuery(
      ReportFormat format) const -> FormattedGroupedReports = 0;
  [[nodiscard]] virtual auto RunExportAllMonthlyReportsQuery(
      ReportFormat format) const -> FormattedMonthlyReports = 0;
  [[nodiscard]] virtual auto RunExportAllPeriodReportsQuery(
      const std::vector<int>& days_list, ReportFormat format) const
      -> FormattedPeriodReports = 0;
  [[nodiscard]] virtual auto RunExportAllWeeklyReportsQuery(
      ReportFormat format) const -> FormattedWeeklyReports = 0;
  [[nodiscard]] virtual auto RunExportAllYearlyReportsQuery(
      ReportFormat format) const -> FormattedYearlyReports = 0;
};

#endif  // APPLICATION_INTERFACES_I_REPORT_QUERY_SERVICE_H_
