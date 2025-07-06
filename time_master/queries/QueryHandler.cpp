#include "QueryHandler.h"

// 引入所有新的报告查询器和格式化器
#include "report_generators/DailyReportQuerier.h"
#include "report_generators/MonthlyReportQuerier.h" 
#include "report_generators/PeriodReportQuerier.h"  

QueryHandler::QueryHandler(sqlite3* db) : m_db(db) {}

std::string QueryHandler::run_daily_query(const std::string& date_str) const {
    DailyReportQuerier querier(m_db, date_str);
    DailyReportData data = querier.fetch_data();
    DailyReportFormatter formatter;
    return formatter.format_report(data, m_db);
}

// 修改月报查询
std::string QueryHandler::run_monthly_query(const std::string& year_month_str) const {
    MonthlyReportQuerier querier(m_db, year_month_str);
    MonthlyReportData data = querier.fetch_data();
    MonthlyReportFormatter formatter;
    return formatter.format_report(data, m_db);
}

// 修改周期报告查询
std::string QueryHandler::run_period_query(int days) const {
    PeriodReportQuerier querier(m_db, days);
    PeriodReportData data = querier.fetch_data();
    PeriodReportFormatter formatter;
    return formatter.format_report(data, m_db);
}