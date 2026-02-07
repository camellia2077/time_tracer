// application/reporting/generator/report_generator.hpp
#ifndef APPLICATION_REPORTING_GENERATOR_REPORT_GENERATOR_H_
#define APPLICATION_REPORTING_GENERATOR_REPORT_GENERATOR_H_

#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "application/interfaces/i_report_query_service.hpp"
#include "domain/reports/models/query_data_structs.hpp"
#include "domain/reports/types/report_format.hpp"

class ReportGenerator {
 public:
  explicit ReportGenerator(std::unique_ptr<IReportQueryService> query_service);
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
  std::unique_ptr<IReportQueryService> query_service_;
};

#endif  // APPLICATION_REPORTING_GENERATOR_REPORT_GENERATOR_H_
