// queries/daily/DayGenerator.cpp
#include "DayGenerator.hpp"
#include "DayQuerier.hpp"
#include "queries/shared/factories/GenericFormatterFactory.hpp" // [修改] 引入新工厂
#include <memory>

DayGenerator::DayGenerator(sqlite3* db, const AppConfig& config)
    : db_(db), app_config_(config) {}

std::string DayGenerator::generate_report(const std::string& date, ReportFormat format) {
    DayQuerier querier(db_, date);
    DailyReportData report_data = querier.fetch_data();

    // [核心修改] 使用新的通用工厂
    auto formatter = GenericFormatterFactory<DailyReportData>::create(format, app_config_);

    return formatter->format_report(report_data);
}