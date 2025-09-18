// queries/monthly/MonthGenerator.cpp
#include "MonthGenerator.hpp"
#include "MonthQuerier.hpp"
#include "queries/shared/factories/FormatterFactory.hpp"
#include <memory>

MonthGenerator::MonthGenerator(sqlite3* db, const AppConfig& config)
    : m_db(db), app_config_(config) {}

std::string MonthGenerator::generate_report(const std::string& year_month, ReportFormat format) {
    MonthQuerier querier(m_db, year_month);
    MonthlyReportData report_data = querier.fetch_data();
    
    auto formatter = FormatterFactory::create_month_formatter(format, app_config_);

    return formatter->format_report(report_data);
}