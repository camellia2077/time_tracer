#include "AllDayReports.h"
#include "queries/daily/DayQuerier.h"
#include "queries/daily/DayFmtFactory.h" // [修正] 添加缺失的头文件
#include <stdexcept>
#include <vector>

AllDayReports::AllDayReports(sqlite3* db) : m_db(db) {
    if (m_db == nullptr) {
        throw std::invalid_argument("Database connection cannot be null.");
    }
}

// 函数签名更新，以接收格式参数
FormattedGroupedReports AllDayReports::generate_all_reports(ReportFormat format) {
    FormattedGroupedReports grouped_reports;
    sqlite3_stmt* stmt;

    const char* sql = "SELECT date, year, month FROM days ORDER BY date ASC;";

    if (sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error("Failed to prepare statement to fetch all dates.");
    }

    // 在循环外使用工厂创建一次格式化器实例，以提高效率
    auto formatter = DayFmtFactory::create_formatter(format);

    // 遍历数据库中的每一天
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const char* date_cstr = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        if (!date_cstr) continue;
        
        std::string date(date_cstr);
        int year = sqlite3_column_int(stmt, 1);
        int month = sqlite3_column_int(stmt, 2);

        // 1. 使用 DayQuerier 获取当天的完整数据
        DayQuerier querier(m_db, date);
        DailyReportData report_data = querier.fetch_data();

        // 2. 使用在循环外创建的格式化器实例来格式化报告
        std::string formatted_report = formatter->format_report(report_data, m_db);

        // 3. 将格式化后的报告存入嵌套的 map 结构中
        grouped_reports[year][month].push_back({date, formatted_report});
    }

    sqlite3_finalize(stmt);
    return grouped_reports;
}