// core/reporting/generator/report_generator.hpp
#ifndef CORE_REPORTING_GENERATOR_REPORT_GENERATOR_HPP_
#define CORE_REPORTING_GENERATOR_REPORT_GENERATOR_HPP_

#include <memory>
#include <string>
#include <vector>
#include "reports/shared/model/query_data_structs.hpp"
#include "reports/shared/types/report_format.hpp"

// [修复] 更新包含路径
#include "common/config/app_config.hpp"

struct sqlite3;
class ReportService;

class ReportGenerator {
public:
    ReportGenerator(sqlite3* db, const AppConfig& config);
    ~ReportGenerator();

    std::string generate_daily_report(const std::string& date, ReportFormat format);
    std::string generate_monthly_report(const std::string& month, ReportFormat format);
    std::string generate_period_report(int days, ReportFormat format);

    FormattedGroupedReports generate_all_daily_reports(ReportFormat format);
    FormattedMonthlyReports generate_all_monthly_reports(ReportFormat format);
    FormattedPeriodReports generate_all_period_reports(const std::vector<int>& days_list, ReportFormat format);


private:
    std::unique_ptr<ReportService> query_handler_;
};

#endif // CORE_REPORTING_GENERATOR_REPORT_GENERATOR_HPP_
