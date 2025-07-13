#include "QueryHandler.h"

// 对于日报，我们直接使用其公共接口 DailyReportGenerator
#include "report_generators/daily/DailyReportGenerator.h" 

// 对于月报和周期报告，我们使用它们的 Querier 和 Formatter
// （因为它们没有提供一个统一的 Generator 类）
#include "report_generators/monthly/MonthlyReportQuerier.h" 
#include "report_generators/period/PeriodReportQuerier.h"  


QueryHandler::QueryHandler(sqlite3* db) : m_db(db) {}

// --- 修改日报查询 ---
// 将实现委托给 DailyReportGenerator
std::string QueryHandler::run_daily_query(const std::string& date_str) const 
{
    // 1. 创建日报生成器 (Generator)
    DailyReportGenerator generator(m_db);

    // 2. 调用其公共接口来生成报告
    // 所有复杂的逻辑（获取数据、格式化）都被封装在 DailyReportGenerator 内部
    return generator.generate_report(date_str);
}


// --- 月报查询保持不变 ---
// 这里的实现是正确的，因为它直接使用了月报的查询器和格式化器
std::string QueryHandler::run_monthly_query(const std::string& year_month_str) const {
    MonthlyReportQuerier querier(m_db, year_month_str);
    MonthlyReportData data = querier.fetch_data();
    MonthlyReportFormatter formatter;
    return formatter.format_report(data, m_db);
}

// --- 周期报告查询保持不变 ---
// 这里的实现也是正确的
std::string QueryHandler::run_period_query(int days) const {
    PeriodReportQuerier querier(m_db, days);
    PeriodReportData data = querier.fetch_data();
    PeriodReportFormatter formatter;
    return formatter.format_report(data, m_db);
}
