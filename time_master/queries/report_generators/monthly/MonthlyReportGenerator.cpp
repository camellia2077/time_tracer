#include "MonthlyReportGenerator.h"

#include "querier/MonthlyReportQuerier.h"
#include "formatter/MonthlyReportFormatter.h"
#include "report_generators/query_data_structs.h"

MonthlyReportGenerator::MonthlyReportGenerator(sqlite3* db) : m_db(db) {}

std::string MonthlyReportGenerator::generate_report(const std::string& year_month) {
    // 1. 使用 Querier 模块获取数据
    MonthlyReportQuerier querier(m_db, year_month);
    MonthlyReportData report_data = querier.fetch_data();

    // 2. 使用 Formatter 模块创建报告字符串
    MonthlyReportFormatter formatter;
    return formatter.format_report(report_data, m_db);
}