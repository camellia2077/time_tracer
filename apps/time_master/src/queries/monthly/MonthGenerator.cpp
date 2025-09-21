// queries/monthly/MonthGenerator.cpp
#include "MonthGenerator.hpp"
#include "MonthQuerier.hpp"
#include "queries/shared/factories/GenericFormatterFactory.hpp" // [修改]
#include <memory>

MonthGenerator::MonthGenerator(sqlite3* db, const AppConfig& config)
    : db_(db), app_config_(config) {}

std::string MonthGenerator::generate_report(const std::string& year_month, ReportFormat format) {
    MonthQuerier querier(db_, year_month);
    MonthlyReportData report_data = querier.fetch_data();
    
    // [核心修改]
    auto formatter = GenericFormatterFactory<MonthlyReportData>::create(format, app_config_);

    return formatter->format_report(report_data);
}