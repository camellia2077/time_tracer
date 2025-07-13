#include "QueryHandler.h"

// 引入所有高级别的 "Generator" 和 "Querier" 类
#include "report_generators/daily/DailyReportGenerator.h" 
// --- [修改] 引入新的月报生成器 ---
#include "report_generators/monthly/MonthlyReportGenerator.h" 
#include "report_generators/period/PeriodReportGenerator.h" 

QueryHandler::QueryHandler(sqlite3* db) : m_db(db) {}

// --- 日报查询保持不变 ---
std::string QueryHandler::run_daily_query(const std::string& date_str) const {
    DailyReportGenerator generator(m_db);
    return generator.generate_report(date_str);
}


// --- [修改] 月报查询，将实现委托给 MonthlyReportGenerator ---
std::string QueryHandler::run_monthly_query(const std::string& year_month_str) const {
    // 1. 创建月报生成器 (Generator)
    MonthlyReportGenerator generator(m_db);
    
    // 2. 调用其公共接口来生成报告
    return generator.generate_report(year_month_str);
}

// --- 周期报告 ---
std::string QueryHandler::run_period_query(int days) const {
    // 1. Create the period report generator
    PeriodReportGenerator generator(m_db);
    
    // 2. Call its public interface to generate the report
    return generator.generate_report(days);
}