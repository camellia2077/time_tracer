// application/reporting/generator/report_generator.hpp
#ifndef APPLICATION_REPORTING_GENERATOR_REPORT_GENERATOR_H_
#define APPLICATION_REPORTING_GENERATOR_REPORT_GENERATOR_H_

#include <memory>
#include <string>
#include <vector>

#include "reports/data/model/query_data_structs.hpp"
#include "reports/shared/types/report_format.hpp"

// [修复] 更新包含路径
#include "common/config/app_config.hpp"

struct sqlite3;
class ReportService;

class ReportGenerator {
 public:
  ReportGenerator(sqlite3* sqlite_db, const AppConfig& config);
  ~ReportGenerator();

  auto GenerateDailyReport(std::string_view date, ReportFormat format)
      -> std::string;
  auto GenerateMonthlyReport(std::string_view month, ReportFormat format)
      -> std::string;
  auto GeneratePeriodReport(int days, ReportFormat format) -> std::string;
  auto GenerateWeeklyReport(std::string_view iso_week, ReportFormat format)
      -> std::string;
  auto GenerateYearlyReport(std::string_view year, ReportFormat format)
      -> std::string;

  auto GenerateAllDailyReports(ReportFormat format) -> FormattedGroupedReports;
  auto GenerateAllMonthlyReports(ReportFormat format)
      -> FormattedMonthlyReports;
  auto GenerateAllPeriodReports(const std::vector<int>& days_list,
                                ReportFormat format) -> FormattedPeriodReports;
  auto GenerateAllWeeklyReports(ReportFormat format) -> FormattedWeeklyReports;
  auto GenerateAllYearlyReports(ReportFormat format) -> FormattedYearlyReports;

 private:
  std::unique_ptr<ReportService> query_handler_;
};

#endif  // APPLICATION_REPORTING_GENERATOR_REPORT_GENERATOR_H_
