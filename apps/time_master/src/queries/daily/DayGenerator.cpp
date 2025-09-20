#include "DayGenerator.hpp"
#include "DayQuerier.hpp"
#include "queries/shared/factories/GenericFormatterFactory.hpp" // [修改] 引入新工厂
#include <memory>

DayGenerator::DayGenerator(sqlite3* db, const AppConfig& config)
    : m_db(db), app_config_(config) {}

std::string DayGenerator::generate_report(const std::string& date, ReportFormat format) {
    DayQuerier querier(m_db, date);
    DailyReportData report_data = querier.fetch_data();

    // [核心修改] 使用新的通用工厂
    auto formatter = GenericFormatterFactory<DailyReportData>::create(format, app_config_);

    return formatter->format_report(report_data);
}