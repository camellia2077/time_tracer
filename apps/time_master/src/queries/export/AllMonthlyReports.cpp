// queries/export/AllMonthlyReports.cpp
#include "AllMonthlyReports.hpp"
#include "queries/monthly/MonthQuerier.hpp"
#include "queries/shared/factories/FormatterFactory.hpp" // [修改] 引入新的统一工厂
#include <vector>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <memory>

AllMonthlyReports::AllMonthlyReports(sqlite3* db, const AppConfig& config) 
    : m_db(db), app_config_(config) {
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

    // [核心修改] 使用统一工厂创建格式化器
    auto formatter = FormatterFactory::create_month_formatter(format, app_config_);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int year = sqlite3_column_int(stmt, 0);
        int month = sqlite3_column_int(stmt, 1);

        std::stringstream year_month_ss;
        year_month_ss << year << std::setw(2) << std::setfill('0') << month;
        std::string year_month_str = year_month_ss.str();

        MonthQuerier querier(m_db, year_month_str);
        MonthlyReportData data = querier.fetch_data();

        if (data.total_duration > 0) {
            std::string formatted_report = formatter->format_report(data);
            reports[year][month] = formatted_report;
        }
    }

    sqlite3_finalize(stmt);
    
    return reports;
}