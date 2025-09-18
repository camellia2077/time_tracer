// queries/daily/DayGenerator.cpp
#include "DayGenerator.hpp"
#include "DayQuerier.hpp"
#include "queries/shared/factories/FormatterFactory.hpp" // [修改] 引入新的统一工厂
#include <memory>

DayGenerator::DayGenerator(sqlite3* db, const AppConfig& config)
    : m_db(db), app_config_(config) {}

std::string DayGenerator::generate_report(const std::string& date, ReportFormat format) {
    DayQuerier querier(m_db, date);
    DailyReportData report_data = querier.fetch_data();

    // [核心修改] 使用统一工厂创建格式化器
    auto formatter = FormatterFactory::create_day_formatter(format, app_config_);

    return formatter->format_report(report_data);
}