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
  ReportGenerator(sqlite3* db, const AppConfig& config);
  ~ReportGenerator();

  std::string generate_daily_report(const std::string& date,
                                    ReportFormat format);
  std::string generate_monthly_report(const std::string& month,
                                      ReportFormat format);
  std::string generate_period_report(int days, ReportFormat format);
  std::string generate_weekly_report(const std::string& iso_week,
                                     ReportFormat format);
  std::string generate_yearly_report(const std::string& year,
                                     ReportFormat format);

  FormattedGroupedReports generate_all_daily_reports(ReportFormat format);
  FormattedMonthlyReports generate_all_monthly_reports(ReportFormat format);
  FormattedPeriodReports generate_all_period_reports(
      const std::vector<int>& days_list, ReportFormat format);
  FormattedWeeklyReports generate_all_weekly_reports(ReportFormat format);
  FormattedYearlyReports generate_all_yearly_reports(ReportFormat format);

 private:
  std::unique_ptr<ReportService> query_handler_;
};

#endif  // APPLICATION_REPORTING_GENERATOR_REPORT_GENERATOR_H_
