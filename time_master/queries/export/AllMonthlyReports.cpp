#include "AllMonthlyReports.h"
// [修改] 引入格式化器工厂和查询器
#include "queries/monthly/MonthlyFormatterFactory.h"
#include "queries/monthly/MonthlyReportQuerier.h"
#include <vector>
#include <iomanip>
#include <sstream>
#include <stdexcept>

AllMonthlyReports::AllMonthlyReports(sqlite3* db) : m_db(db) {
    if (m_db == nullptr) {
        throw std::invalid_argument("Database connection cannot be null.");
    }
}

// [修改] 更新函数签名以接收 ReportFormat 参数
FormattedMonthlyReports AllMonthlyReports::generate_reports(ReportFormat format) {
    FormattedMonthlyReports reports;
    sqlite3_stmt* stmt;

    const char* sql = "SELECT DISTINCT year, month FROM days ORDER BY year, month;";
    if (sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error("Failed to prepare statement to fetch unique year/month pairs.");
    }

    // [修改] 在循环外使用工厂创建一次格式化器实例，以供复用
    auto formatter = MonthlyFormatterFactory::create_formatter(format);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int year = sqlite3_column_int(stmt, 0);
        int month = sqlite3_column_int(stmt, 1);

        std::stringstream year_month_ss;
        year_month_ss << year << std::setw(2) << std::setfill('0') << month;
        std::string year_month_str = year_month_ss.str();

        // 1. 获取数据 (逻辑不变)
        MonthlyReportQuerier querier(m_db, year_month_str);
        MonthlyReportData data = querier.fetch_data();

        if (data.total_duration > 0) {
            // 2. [修改] 使用在循环外创建的格式化器实例来格式化报告
            std::string formatted_report = formatter->format_report(data, m_db);

            // 3. 存储结果 (逻辑不变)
            reports[year][month] = formatted_report;
        }
    }

    sqlite3_finalize(stmt);
    
    return reports;
}