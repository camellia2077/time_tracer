#include "QueryHandler.h"

// 引入所有高级别的 "Generator" 类
#include "report_generators/daily/DailyReportGenerator.h" 
#include "report_generators/monthly/MonthlyReportGenerator.h" 
#include "report_generators/period/PeriodReportGenerator.h"
#include "report_generators/AllDayReports.h" // [新增] 引入新的“导出所有报告”生成器

QueryHandler::QueryHandler(sqlite3* db) : m_db(db) {}

// --- 日报查询保持不变 ---
std::string QueryHandler::run_daily_query(const std::string& date_str) const {
    DailyReportGenerator generator(m_db);
    return generator.generate_report(date_str);
}


// --- 月报查询 ---
std::string QueryHandler::run_monthly_query(const std::string& year_month_str) const {
    MonthlyReportGenerator generator(m_db);
    return generator.generate_report(year_month_str);
}

// --- 周期报告 ---
std::string QueryHandler::run_period_query(int days) const {
    PeriodReportGenerator generator(m_db);
    return generator.generate_report(days);
}

// --- [新增] 导出所有日报内容的实现 ---
FormattedGroupedReports QueryHandler::run_export_all_reports_query() const {
    AllReportsGenerator generator(m_db);
    return generator.generate_all_reports();
}
