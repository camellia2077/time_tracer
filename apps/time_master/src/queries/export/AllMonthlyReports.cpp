// queries/export/AllMonthlyReports.cpp

#include "AllMonthlyReports.hpp"
#include "queries/monthly/MonthQuerier.hpp"
#include <vector>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <memory>

// [修改] 引入新的通用工厂和具体的格式化器类
#include "queries/shared/factories/FmtFactory.hpp"
#include "queries/monthly/formatters/md/MonthMd.hpp"
#include "queries/monthly/formatters/tex/MonthTex.hpp"
#include "queries/monthly/formatters/typ/MonthTyp.hpp"
#include "queries/monthly/formatters/typ/MonthTypConfig.hpp" // [新增] 引入月报Typst配置类

// [修改] 构造函数，接收数据库连接和月报Typst配置路径
AllMonthlyReports::AllMonthlyReports(sqlite3* db, const std::string& month_typ_config_path)
    : m_db(db), m_month_typ_config_path(month_typ_config_path) {
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

    std::unique_ptr<IReportFormatter<MonthlyReportData>> formatter;
    switch (format) {
        case ReportFormat::Typ: {
            auto config = std::make_shared<MonthTypConfig>(m_month_typ_config_path);
            formatter = std::make_unique<MonthTyp>(config);
            break;
        }
        case ReportFormat::Markdown: {
            formatter = std::make_unique<MonthMd>();
            break;
        }
        case ReportFormat::LaTeX: {
            formatter = std::make_unique<MonthTex>();
            break;
        }
    }

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