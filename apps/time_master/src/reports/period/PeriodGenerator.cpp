// reports/period/PeriodGenerator.cpp
#include "reports/period/PeriodGenerator.hpp"
#include "reports/period/PeriodQuerier.hpp"
#include "reports/shared/factories/GenericFormatterFactory.hpp" // [修改]
#include <memory>

PeriodGenerator::PeriodGenerator(sqlite3* db, const AppConfig& config)
    : db_(db), app_config_(config) {}

std::string PeriodGenerator::generate_report(int days, ReportFormat format) {
    PeriodQuerier querier(db_, days);
    PeriodReportData report_data = querier.fetch_data();

    // [核心修改]
    auto formatter = GenericFormatterFactory<PeriodReportData>::create(format, app_config_);

    return formatter->format_report(report_data);
}