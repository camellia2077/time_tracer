#include "QueryHandler.h"
#include "queries/daily/DailyReportGenerator.h" 
#include "queries/monthly/MonthlyReportGenerator.h" 
#include "queries/period/PeriodReportGenerator.h"
#include "queries/export/AllDayReports.h"
#include "queries/export/AllMonthlyReports.h"
#include "queries/export/AllPeriodReports.h"

QueryHandler::QueryHandler(sqlite3* db) : m_db(db) {}

std::string QueryHandler::run_daily_query(const std::string& date_str, ReportFormat format) const {
    DailyReportGenerator generator(m_db);
    return generator.generate_report(date_str, format);
}

std::string QueryHandler::run_monthly_query(const std::string& year_month_str, ReportFormat format) const {
    MonthlyReportGenerator generator(m_db);
    return generator.generate_report(year_month_str, format);
}

// [修改] 更新函数以传递 format 参数
std::string QueryHandler::run_period_query(int days, ReportFormat format) const {
    PeriodReportGenerator generator(m_db);
    // 将 format 参数传递给周期报告生成器
    return generator.generate_report(days, format);
}

FormattedGroupedReports QueryHandler::run_export_all_daily_reports_query(ReportFormat format) const {
    AllDayReports generator(m_db);
    return generator.generate_all_reports(format);
}

FormattedMonthlyReports QueryHandler::run_export_all_monthly_reports_query(ReportFormat format) const {
    AllMonthlyReports generator(m_db);
    return generator.generate_reports(format);
}

// [修改] 更新函数以传递 format 参数
FormattedPeriodReports QueryHandler::run_export_all_period_reports_query(const std::vector<int>& days_list, ReportFormat format) const {
    AllPeriodReports generator(m_db);
    // 将 format 参数传递给批量周期报告生成器
    return generator.generate_reports(days_list, format);
}