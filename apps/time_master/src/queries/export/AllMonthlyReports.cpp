// queries/export/AllMonthlyReports.cpp

#include "AllMonthlyReports.hpp"
#include "queries/monthly/MonthQuerier.hpp"
#include <vector>
#include <iomanip>
#include <sstream>
#include <stdexcept>

// [修改] 引入新的通用工厂和具体的格式化器类
#include "queries/shared/factories/FmtFactory.hpp"
#include "queries/monthly/formatters/md/MonthMd.hpp"
#include "queries/monthly/formatters/tex/MonthTex.hpp"
#include "queries/monthly/formatters/typ/MonthTyp.hpp"

AllMonthlyReports::AllMonthlyReports(sqlite3* db) : m_db(db) {
    if (m_db == nullptr) {
        throw std::invalid_argument("Database connection cannot be null.");
    }
}

FormattedMonthlyReports AllMonthlyReports::generate_reports(ReportFormat format) {
    FormattedMonthlyReports reports;
    sqlite3_stmt* stmt;

    const char* sql = "SELECT DISTINCT year, month FROM days ORDER BY year, month;";
    if (sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error("Failed to prepare statement to fetch unique year/month pairs.");
    }

    // [修改] 使用新的模板工厂创建格式化器
    auto formatter = ReportFmtFactory<MonthlyReportData, MonthMd, MonthTex, MonthTyp>::create_formatter(format);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int year = sqlite3_column_int(stmt, 0);
        int month = sqlite3_column_int(stmt, 1);

        std::stringstream year_month_ss;
        year_month_ss << year << std::setw(2) << std::setfill('0') << month;
        std::string year_month_str = year_month_ss.str();

        MonthQuerier querier(m_db, year_month_str);
        MonthlyReportData data = querier.fetch_data();

        if (data.total_duration > 0) {
            std::string formatted_report = formatter->format_report(data, m_db);
            reports[year][month] = formatted_report;
        }
    }

    sqlite3_finalize(stmt);
    
    return reports;
}