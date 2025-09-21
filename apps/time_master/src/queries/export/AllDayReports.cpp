// queries/export/AllDayReports.cpp
#include "AllDayReports.hpp"
#include "queries/daily/DayQuerier.hpp"
#include "queries/shared/factories/GenericFormatterFactory.hpp" // [修改]
#include <stdexcept>
#include <vector>
#include <memory>

AllDayReports::AllDayReports(sqlite3* db, const AppConfig& config)
    : db_(db), app_config_(config) {
    if (db_ == nullptr) {
        throw std::invalid_argument("Database connection cannot be null.");
    }
}

FormattedGroupedReports AllDayReports::generate_all_reports(ReportFormat format) {
    FormattedGroupedReports grouped_reports;
    sqlite3_stmt* stmt;

    const char* sql = "SELECT date, year, month FROM days ORDER BY date ASC;";

    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error("Failed to prepare statement to fetch all dates.");
    }

    // [核心修改]
    auto formatter = GenericFormatterFactory<DailyReportData>::create(format, app_config_);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const char* date_cstr = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        if (!date_cstr) continue;

        std::string date(date_cstr);
        int year = sqlite3_column_int(stmt, 1);
        int month = sqlite3_column_int(stmt, 2);

        DayQuerier querier(db_, date);
        DailyReportData report_data = querier.fetch_data();

        std::string formatted_report = formatter->format_report(report_data);
        grouped_reports[year][month].push_back({date, formatted_report});
    }

    sqlite3_finalize(stmt);
    return grouped_reports;
}