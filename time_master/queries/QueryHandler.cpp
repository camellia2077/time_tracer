#include "QueryHandler.h"
#include "report_generators/daily/DailyReportGenerator.h" 
#include "report_generators/monthly/MonthlyReportGenerator.h" 
#include "report_generators/period/PeriodReportGenerator.h"
#include "report_generators/AllDayReports.h"
#include "report_generators/AllMonthlyReports.h"

QueryHandler::QueryHandler(sqlite3* db) : m_db(db) {}

std::string QueryHandler::run_daily_query(const std::string& date_str) const {
    DailyReportGenerator generator(m_db);
    return generator.generate_report(date_str);
}

std::string QueryHandler::run_monthly_query(const std::string& year_month_str) const {
    MonthlyReportGenerator generator(m_db);
    return generator.generate_report(year_month_str);
}

std::string QueryHandler::run_period_query(int days) const {
    PeriodReportGenerator generator(m_db);
    return generator.generate_report(days);
}

FormattedGroupedReports QueryHandler::run_export_all_daily_reports_query() const {
    AllDayReports generator(m_db);
    return generator.generate_all_reports();
}

// [新增] 导出所有月报的实现
FormattedMonthlyReports QueryHandler::run_export_all_monthly_reports_query() const {
    AllMonthlyReports generator(m_db);
    return generator.generate_reports();
}
