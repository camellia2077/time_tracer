// reports/services/AllPeriodReports.cpp
#include "AllPeriodReports.hpp"
#include "reports/period/PeriodQuerier.hpp"
#include "reports/shared/factories/GenericFormatterFactory.hpp" // [修改]
#include <stdexcept>
#include <memory>

AllPeriodReports::AllPeriodReports(sqlite3* db, const AppConfig& config) 
    : db_(db), app_config_(config) {
    if (db_ == nullptr) {
        throw std::invalid_argument("Database connection cannot be null.");
    }
}

FormattedPeriodReports AllPeriodReports::generate_reports(const std::vector<int>& days_list, ReportFormat format) {
    FormattedPeriodReports reports;

    // [核心修改]
    auto formatter = GenericFormatterFactory<PeriodReportData>::create(format, app_config_);

    for (int days : days_list) {
        if (days > 0) {
            PeriodQuerier querier(db_, days);
            PeriodReportData report_data = querier.fetch_data();

            std::string formatted_report = formatter->format_report(report_data);
            reports[days] = formatted_report;
        }
    }

    return reports;
}