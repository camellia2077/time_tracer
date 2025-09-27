#ifndef REPORT_GENERATOR_HPP
#define REPORT_GENERATOR_HPP

#include <memory>
#include <string>
#include <vector>
#include "queries/shared/data/query_data_structs.hpp"
#include "queries/shared/types/ReportFormat.hpp"
#include "common/AppConfig.hpp"

struct sqlite3;
class QueryHandler;

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
    std::unique_ptr<QueryHandler> query_handler_;
};

#endif // REPORT_GENERATOR_HPP