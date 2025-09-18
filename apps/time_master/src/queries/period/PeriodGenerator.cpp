// queries/period/PeriodGenerator.cpp
#include "queries/period/PeriodGenerator.hpp"
#include "queries/period/PeriodQuerier.hpp"
#include "queries/shared/factories/FormatterFactory.hpp" // [修改] 引入新的统一工厂
#include <memory>

PeriodGenerator::PeriodGenerator(sqlite3* db, const AppConfig& config)
    : m_db(db), app_config_(config) {}

std::string PeriodGenerator::generate_report(int days, ReportFormat format) {
    PeriodQuerier querier(m_db, days);
    PeriodReportData report_data = querier.fetch_data();

    // [核心修改] 使用统一工厂创建格式化器
    auto formatter = FormatterFactory::create_period_formatter(format, app_config_);

    return formatter->format_report(report_data);
}