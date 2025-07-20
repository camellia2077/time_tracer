#include "QueryHandler.h"
#include "report_generators/daily/DailyReportGenerator.h" 
#include "report_generators/monthly/MonthlyReportGenerator.h" 
#include "report_generators/period/PeriodReportGenerator.h"
#include "report_generators/AllDayReports.h"
#include "report_generators/AllMonthlyReports.h"
#include "report_generators/AllPeriodReports.h"

QueryHandler::QueryHandler(sqlite3* db) : m_db(db) {}

// [修改] 实现更新后的接口，将 format 参数传递下去
std::string QueryHandler::run_daily_query(const std::string& date_str, ReportFormat format) const {
    DailyReportGenerator generator(m_db);
    return generator.generate_report(date_str, format);
}

std::string QueryHandler::run_monthly_query(const std::string& year_month_str) const {
    MonthlyReportGenerator generator(m_db);
    return generator.generate_report(year_month_str);
}

std::string QueryHandler::run_period_query(int days) const {
    PeriodReportGenerator generator(m_db);
    return generator.generate_report(days);
}

// [修改] 实现更新后的接口，将 format 参数传递下去
FormattedGroupedReports QueryHandler::run_export_all_daily_reports_query(ReportFormat format) const {
    AllDayReports generator(m_db);
    return generator.generate_all_reports(format);
}

FormattedMonthlyReports QueryHandler::run_export_all_monthly_reports_query() const {
    AllMonthlyReports generator(m_db);
    return generator.generate_reports();
}

FormattedPeriodReports QueryHandler::run_export_all_period_reports_query(const std::vector<int>& days_list) const {
    AllPeriodReports generator(m_db);
    return generator.generate_reports(days_list);
}